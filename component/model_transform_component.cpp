//
// Created by 潘鑫 on 2026/3/18.
//

#include "model_transform_component.h"

#include "camera_optical_component.h"
#include "input_component.h"
#include "vulkan_texture_bindless.h"
#include "base_geometry/intersect_function.h"
#include "manifold/linalg.h"

Ray<Point_3> &get_screen_ray(const Point_2 mouse_positon) {
    static Point_2 last_mouse_position = {0, 0};
    static Ray<Point_3> last_ray       = {{0, 0, 0}, {0, 0, 1}};
    if (mouse_positon == last_mouse_position) {
        return last_ray;
    }
    last_mouse_position = mouse_positon;
    // 上面是一个简短记忆上一次的代码

    auto world_entity     = get_world_root();
    auto camera           = Logic_entt().try_get<camera_optical_component>(world_entity);
    const auto camera_pos = Logic_entt().try_get<model_transform>(world_entity);

    const auto &backend    = VK_backend::get();
    auto [width, height]   = backend.get_current_extent();
    const auto projection  = camera->get_projection();
    const auto view_matrix = camera_pos->get_view_projection();

    // 1. 转换到 NDC 坐标 (假设鼠标坐标为 mouseX, mouseY)
    // 这里有一个坑，gltf 给出的坐标和拿到的 显示区域的宽和高差两倍
    float x = (4.0f * mouse_positon.x) / width - 1.0f;
    float y = (4.0f * mouse_positon.y) / height - 1.0f; // 注意：Vulkan/GLFW 的 Y 轴通常需要反转

    // 这里是什么空间？
    LOG_INFO(g_log(), "NDC x: {} y: {}", x, y);
    // 2. 构造近裁剪面和远裁剪面的点 (在裁剪空间)
    // Vulkan 的近平面通常是 z=0.0，远平面是 z=1.0
    Eigen::Vector4f ray_start_clip(x, y, 0.0f, 1.0f);
    Eigen::Vector4f ray_end_clip(x, y, 1.0f, 1.0f);

    // 3. 计算逆矩阵
    Eigen::Matrix4f invVP = (projection * view_matrix).inverse();

    // 4. 转换回世界空间
    Eigen::Vector4f world_start = invVP * ray_start_clip; // 这里给出来的是近平面上的起始点
    Eigen::Vector4f world_end   = invVP * ray_end_clip;

    // 5. 透视除法 (W 分量归一化)
    world_start /= world_start.w();
    world_end   /= world_end.w();
    auto offset = camera_pos->get_offset(); // 这里给出的相机的位置
    // 6. 确定射线
    Eigen::Vector3f ray_origin    = world_start.head<3>();
    Eigen::Vector3f ray_direction = (world_end.head<3>() - ray_origin).normalized();
    last_ray                      = {
        {offset},
        {ray_direction.x(), ray_direction.y(), ray_direction.z()}
    };
    return last_ray;
}


wmOperatorStatus model_3d_Event(const entt::entity entity, const base_event_with_stamp &event) {
    auto temp_type = event.event_type;
    auto &status   = Logic_entt().get<Input_Component>(entity);

    switch (temp_type) {
        case MOUSE_ROTATE: {
            auto temp = event.scroll;
            // 绕 Z 轴旋转 45 度

            if (auto position = Logic_entt().try_get<model_transform>(entity)) {
                auto q_current = position->get_rotate();
                q_current = Eigen::Quaternionf(Eigen::AngleAxisf(temp.x / 100, Eigen::Vector3f::UnitY())) * q_current;
                q_current = Eigen::Quaternionf(Eigen::AngleAxisf(temp.y / 100, Eigen::Vector3f::UnitX())) * q_current;

                position->set_rotate(q_current);
                Logic_entt().emplace_or_replace<UI_transform_dirty>(entity);
            }
            return OPERATOR_RUNNING_MODAL;
        }
        case EVT_KEY_X:
            // 删除当前鼠标位置的元素
            if (event.event_code == KM_PRESS)
                if (Logic_entt().valid(entity)) {
                    Logic_entt().emplace_or_replace<Logic_destroy_tag>(entity);
                    return OPERATOR_FINISHED;
                }
            return OPERATOR_PASS_THROUGH;
            break;
        case EVT_KEY_ESCAPE:
            if (event.event_code == KM_PRESS) {
                // std::cout << " button  EVT_KEY_ESCAPE KM_RELEASE" << std::endl;
                // 需要增加模态的处理 返回结束模态 先用按下的状态，之后再更改
                return OPERATOR_CANCELLED;
            }
            break;
        case MOUSE_LEFT:
            if (event.event_code == KM_PRESS) {
                status.select_status_ = select_current;
                // std::cout << " button  MOUSE_LEFT KM_PRESS" << std::endl;
                // 需要增加模态的处理 返回锁定模态
                return OPERATOR_RUNNING_MODAL;
            }
            if (event.event_code == KM_RELEASE) {
                // std::cout << " button  MOUSE_LEFT KM_RELEASE" << std::endl;
                // 需要增加模态的处理 返回结束模态
                // auto block_entity = UI_button("新按钮", 10, 10, 220, 220);
                status.select_status_ = no_select_current;
                return OPERATOR_FINISHED;
            }
            break;
        case MOUSE_RIGHT:
            break;
        case WHEEL_UP_MOUSE:
            if (auto *transform = Logic_entt().try_get<model_transform>(entity)) {
            }
            break;
        case MOUSE_MOVE:
            if (status.select_status_ == select_current) {
                if (auto *transform = Logic_entt().try_get<model_transform>(entity)) {
                    auto object_offset = transform->get_offset();
                    auto world_entity  = get_world_root();
                    auto ray           = get_screen_ray(event.current_position);
                    auto ray_2         = get_screen_ray(event.last_position);

                    const auto camera_pos = Logic_entt().try_get<model_transform>(world_entity);

                    auto q            = camera_pos->get_rotate();
                    Eigen::Vector3f n = q * Eigen::Vector3f::UnitZ(); // 假设法向量指向 Z 轴
                    n.normalize();
                    auto a = intersect_result({object_offset, {n.x(), n.y(), n.z()}}, ray);
                    auto b = intersect_result({object_offset, {n.x(), n.y(), n.z()}}, ray_2);

                    transform->add_offset(a - b);
                    // 这里 y 需要乘与一个 负号的 原因是因为 拿到的 屏幕的坐标 与 归一化坐标不一致
                    Logic_entt().emplace_or_replace<UI_transform_dirty>(entity);
                    return OPERATOR_RUNNING_MODAL;
                }
            }
            break;
        default:
            return OPERATOR_PASS_THROUGH;
    }
    return OPERATOR_HANDLED;
}

void update_camera_parameter(entt::entity entity) {
    auto camera                      = Logic_entt().get_or_emplace<camera_optical_component>(entity);
    const auto projection            = camera.get_projection();
    const auto inv_projection_matrix = projection.inverse();
    const Point_3 world_light_pos{0, 10, 6};

    const auto camera_pos = Logic_entt().get_or_emplace<model_transform>(entity, Point_3{
                                                                             0, 0, 6
                                                                         });
    const auto view_matrix     = camera_pos.get_view_projection();
    Point_3 world_camera_pos   = camera_pos.get_offset();
    const auto inv_view_matrix = view_matrix.inverse();

    Eigen::Matrix4f invVP   = (projection * view_matrix).inverse();
    Eigen::Matrix4f invVP_3 = inv_view_matrix * inv_projection_matrix;


    set_render_parameter(entity, "global_projection_4x4", projection);
    set_render_parameter(entity, "global_inv_projection_4x4", inv_projection_matrix);


    set_render_parameter(entity, "global_view_4x4", view_matrix);
    set_render_parameter(entity, "global_ins_view_4x4", inv_view_matrix);
    set_render_parameter(entity, "global_world_view_Pos", world_camera_pos);
    set_render_parameter(entity, "global_inv_VP", invVP);

    set_render_parameter(entity, "global_world_light_Pos", world_light_pos);
}


void init_world_scene_root(entt::entity entity) {
    Logic_entt().emplace<Scene_Component>(entity);
    Logic_entt().emplace<Name_component>(entity, "world_scene_root");
    Logic_entt().emplace<VKR_shader_paths>(entity,
                                           "/Users/panxin/CLionProjects/hello_mac/render/shader/multiple_render_targets.vert.spv",
                                           "/Users/panxin/CLionProjects/hello_mac/render/shader/multiple_render_targets.frag.spv",
                                           "", "");


    update_camera_parameter(entity);
    // // vec2 ndc = in_UV * 2.0 - 1.0;
    //
    // // 2. 计算视图空间中的目标点 (设 z=1 为远裁剪面方向)
    // auto viewTarget            = (inv_view_matrix * inv_projection_matrix * Eigen::Vector4f(0.0f, -1.0f, 1.0, 1.0));
    // auto far_x                 = viewTarget.x() / viewTarget.w();
    // auto far_y                 = viewTarget.y() / viewTarget.w();
    // auto far_z                 = viewTarget.z() / viewTarget.w();
    // auto viewTarget_normalized = Eigen::Vector3f(far_x, far_y, far_z).normalized();
    //
    //
    // auto ray_dir_x = far_x - world_camera_pos.x;
    // auto ray_dir_y = far_y - world_camera_pos.y;
    // auto ray_dir_z = far_z - world_camera_pos.z;
    //
    // auto pow = std::sqrt(ray_dir_x * ray_dir_x + ray_dir_y * ray_dir_y + ray_dir_z * ray_dir_z);
    //
    // Point_3 ray_dir{ray_dir_x / pow, ray_dir_y / pow, ray_dir_z / pow};

    allocate_descriptor_sets(entity, "bindless"); // todo : 需要确定放在哪里？
}


uint32_t free_bindless_uniform_sampler2D(const std::string &name) {
    const auto world_entity = world_scene_root::get();
    auto &bindless          = Logic_entt().get_or_emplace<bindless_uniform_sampler2D>(world_entity);
    const auto it_offset    = bindless.bindings.find(name);
    if (it_offset != bindless.bindings.end()) {
        bindless.freeSlots.emplace(it_offset->second.first);
        bindless.bindings.erase(it_offset);
    } else {
    }
}

uint32_t add_bindless_uniform_sampler2D(const std::string &name,
                                        std::optional<Texture_parameter> &update) {
    const auto world_entity = world_scene_root::get();
    auto &bindless          = Logic_entt().get_or_emplace<bindless_uniform_sampler2D>(world_entity);
    auto return_value       = bindless.bindings.size() + bindless.freeSlots.size(); // free为空时，在最大值处更新
    if (!bindless.freeSlots.empty()) {
        // 不为空时，拿取 队列中的 第一个被释放的 slot
        return_value = bindless.freeSlots.front();
        bindless.freeSlots.pop();
    }
    if (const auto shader_temp = Logic_entt().try_get<VKR_shader_paths>(world_entity)) {
        if (!Logic_entt().all_of<std::shared_ptr<vk_shader_data> >(world_entity)) {
            Logic_entt().emplace<std::shared_ptr<vk_shader_data> >(world_entity, VKR_shader_init(*shader_temp));
        }
        const auto &shader_data = Logic_entt().get<std::shared_ptr<vk_shader_data> >(world_entity);
        auto &parameter         = Logic_entt().get_or_emplace<Parameter_used>(world_entity);

        for (auto const &[set_value, bindings_map]: shader_data->bindless_sets_bindings) {
            for (const auto &[binding_value, info]: bindings_map) {
                if (info.binding_name == "bindless_samplerColorMap") {
                    Update_descriptor_binding temp                  = {};
                    temp.binding_name                               = "bindless_samplerColorMap";
                    temp.resource_type                              = "uniform sampler2D";
                    temp.dstSet                                     = set_value; // get 不用 get 了
                    temp.descriptor_write_binding.sType             = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    temp.descriptor_write_binding.dstBinding        = binding_value;
                    temp.descriptor_write_binding.dstArrayElement   = bindless.bindings.size();
                    temp.descriptor_write_binding.descriptorCount   = 1;
                    temp.descriptor_write_binding.pBufferInfo       = nullptr;
                    temp.descriptor_write_binding.pImageInfo        = nullptr;
                    temp.descriptor_write_binding.pTexelBufferView  = nullptr;
                    temp.descriptor_write_binding.descriptorType    = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    temp.texture_info                               = {true, update.value()};
                    parameter.update_bindless_descriptor_sets[name] = temp;
                    // 这个时候需要做什么呢？ 添加一个更新的函数，这是记录了需要更新的内容，还没有真正更新
                    bindless.bindings[name] = {return_value, temp};
                }
            }
        }
    }
    add_bindless_update_tag();
    return return_value;
}


void update_camera_transform() {
    const auto view = Logic_entt().view<Camera_transform_dirty, Name_component, model_transform>();
    for (const auto it: view) {
        auto &camera_pos = view.get<model_transform>(it);
        auto &name       = view.get<Name_component>(it);
        if (name.name_.find("world_scene_root") != std::string::npos) {
            update_camera_parameter(it);
            auto lambda = [](const entt::entity entity) {
                if (Logic_entt().all_of<Scene_Component>(entity))
                    Logic_entt().emplace_or_replace<UI_transform_dirty>(entity);
            };
            add_recursion_function_to_children(it, lambda);
        }
        Logic_entt().remove<Camera_transform_dirty>(it);
    }
}
