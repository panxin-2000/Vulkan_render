//
// Created by 潘鑫 on 2026/3/18.
//

#include "transform_component.h"

#include "camera_optical_component.h"
#include "input_component.h"
#include "vulkan_texture_bindless.h"
#include "base_geometry/intersect_function.h"
#include "manifold/linalg.h"
#include "move_speed.h"
#include "world_scene_root.h"

/**
 * 使用位置和四元数构建 View 矩阵
 * 适配 Vulkan (列优先)
 */
Eigen::Matrix4f view_matrix(const Eigen::Vector3f &pos, const Eigen::Quaternionf &q) {
    // 1. 将四元数转换为旋转矩阵（Eigen 会自动处理归一化并使用 NEON 加速）
    // 注意：View 矩阵需要的是相机的逆旋转
    Eigen::Matrix3f R = q.toRotationMatrix().transpose();

    // 2. 计算平移部分：-(R * pos)
    Eigen::Vector3f t = -(R * pos);

    // 3. 组合成 4x4 矩阵
    Eigen::Matrix4f view   = Eigen::Matrix4f::Identity();
    view.block<3, 3>(0, 0) = R;
    view.block<3, 1>(0, 3) = t;

    return view;
}


[[nodiscard]] Eigen::Matrix4f get_model_matrix(const Transform transform) {
    return transform.get_transform_matrix();
}

Render_AABB transform_AABB(const Render_AABB &bound_box, const Eigen::Matrix4f &matrix) {
    const Eigen::Vector4f new_centroid  = matrix * bound_box.centroid_points;
    const Eigen::Matrix3f R             = matrix.block<3, 3>(0, 0);
    const Eigen::Vector3f new_direction = R.cwiseAbs() * bound_box.direction_intervals.head<3>();
    return {
        {new_centroid.x(), new_centroid.y(), new_centroid.z(), 1.0f},
        {new_direction.x(), new_direction.y(), new_direction.z(), 0.0f}
    };
}


void update_transform_matrix(const entt::entity entity) {
    if (Logic_entt().all_of<Transform, Scene_Component, Transform_matrix_dirty>(entity)) {
        // 满足条件：两个组件都有
        auto parent_entity           = get_parent(entity);
        auto parent_transform_matrix = Logic_entt().get_or_emplace<Transform_Matrix>(parent_entity,
                 Eigen::Matrix4f::Identity());
        const auto &transform        = Logic_entt().get<Transform>(entity);
        const Eigen::Matrix4f result = parent_transform_matrix * transform.get_transform_matrix();
        Logic_entt().emplace_or_replace<Transform_Matrix>(entity, result);
        if (Logic_entt().all_of<Transform_Matrix, Local_Space_AABB>(entity)) {
            const auto aabb = Logic_entt().get<Local_Space_AABB>(entity);
            const auto temp = transform_AABB(aabb, result);
            Logic_entt().emplace_or_replace<World_Space_AABB>(entity, temp.get_aabb_min());
        }
        Logic_entt().remove<Transform_matrix_dirty>(entity);
    }
};


void set_transform_dirty(const entt::entity entity) {
    if (Logic_entt().all_of<Scene_Component>(entity)) {
        Logic_entt().emplace_or_replace<Transform_matrix_dirty>(entity);
    }
}

// [[nodiscard]] Eigen::Matrix4f get_model_matrix(const AABB_min_max<Point_3> &bound_box, const Transform transform) {
//     Eigen::Affine3f model_4x4 = Eigen::Affine3f::Identity();
//     const auto center         = bound_box.get_centroid();
//     model_4x4.translate(Eigen::Vector3f(center.x, center.y, center.z));
//     Eigen::Affine3f model_4x4_2 = Eigen::Affine3f::Identity();
//     model_4x4_2.translate(Eigen::Vector3f(-center.x, -center.y, -center.z));
//     Eigen::Matrix4f result = model_4x4.matrix() * transform.get_transform_matrix() * model_4x4_2.matrix();
//     return result;
// }


Ray<Eigen::Vector3f> &get_screen_ray(const Eigen::Vector2f mouse_positon) {
    static Eigen::Vector2f last_mouse_position = {0, 0};
    static Ray<Eigen::Vector3f> last_ray       = {{0, 0, 0}, {0, 0, 1}};
    if (mouse_positon == last_mouse_position) {
        return last_ray;
    }
    last_mouse_position = mouse_positon;
    // 上面是一个简短记忆上一次的代码

    auto world_entity = get_world_root();
    auto camera       = Logic_entt().try_get<camera_optical_component>(world_entity);

    const auto &backend = VK_backend::instance();
    int width, height;
    SDL_GetWindowSize(backend.get_window(), &width, &height);
    const auto projection  = camera->get_projection_matrix();
    const auto view_matrix = camera->get_view_matrix();

    // 1. 转换到 NDC 坐标 (假设鼠标坐标为 mouseX, mouseY)
    // 这里有一个坑，gltf 给出的坐标和拿到的 显示区域的宽和高差两倍
    float x = (2.0f * mouse_positon.x()) / static_cast<float>(width) - 1.0f;
    float y = (2.0f * mouse_positon.y()) / static_cast<float>(height) - 1.0f; // 注意：Vulkan/GLFW 的 Y 轴通常需要反转
    // 应该更改为 2 因为上面的  SDL_GetWindowSize 的大小改变了， 是屏幕的逻辑大小，而不是具体的像素大小
    // 这里是什么空间？
    // LOG_INFO(g_log(), "NDC x: {} y: {}", x, y);
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
    auto offset = camera->get_position(); // 这里给出的相机的位置
    // 6. 确定射线
    Eigen::Vector3f ray_origin    = world_start.head<3>();
    Eigen::Vector3f ray_direction = (world_end.head<3>() - ray_origin).normalized();
    last_ray                      = {
        {offset.x(), offset.y(), offset.z()},
        {ray_direction.x(), ray_direction.y(), ray_direction.z()}
    };
    return last_ray; // {offset, ray_direction};
}


wmOperatorStatus model_3d_Event(const entt::entity entity, const SDL_Event &event) {
    auto &status = Logic_entt().get<Input_Component>(entity);
    switch (event.type) {
        case SDL_EVENT_MOUSE_WHEEL: {
            Point_2 temp;
            if (std::abs(event.wheel.x) > std::abs(event.wheel.y))
                temp = {-event.wheel.x, 0};
            else
                temp = {0, event.wheel.y};
            if (auto position = Logic_entt().try_get<Transform>(entity)) {
                auto q_current = position->get_rotate();
                auto delta_y   = Eigen::Quaternionf(Eigen::AngleAxisf(temp.x / 100, Eigen::Vector3f::UnitY()));
                auto delta_x   = Eigen::Quaternionf(Eigen::AngleAxisf(temp.y / 100, Eigen::Vector3f::UnitX()));
                position->set_rotate(delta_x * delta_y * q_current);
                Logic_entt().emplace_or_replace<UI_transform_dirty>(entity);
            }
            return OPERATOR_RUNNING_MODAL;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            status.select_status_ = select_current;
            return OPERATOR_RUNNING_MODAL;
        }
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            status.select_status_ = no_select_current;
            return OPERATOR_FINISHED;
        }
        case SDL_EVENT_TEXT_INPUT: {
            break;
        }
        case SDL_EVENT_KEY_UP: {
            break;
        }
        case SDL_EVENT_KEY_DOWN: {
            break;
            if (event.key.key == SDLK_W && Logic_entt().valid(entity)) {
                if (auto position = Logic_entt().try_get<Transform>(entity)) {
                    position->add_offset({0, 0, -1});
                    Logic_entt().emplace_or_replace<Camera_dirty>(entity);
                }
                return OPERATOR_FINISHED;
            } else if (event.key.key == SDLK_S && Logic_entt().valid(entity)) {
                if (auto position = Logic_entt().try_get<Transform>(entity)) {
                    position->add_offset({0, 0, 1});
                    Logic_entt().emplace_or_replace<Camera_dirty>(entity);
                }
                return OPERATOR_FINISHED;
            } else if (event.key.key == SDLK_A && Logic_entt().valid(entity)) {
                if (auto position = Logic_entt().try_get<Transform>(entity)) {
                    position->add_offset({-1, 0, 0});
                    Logic_entt().emplace_or_replace<Camera_dirty>(entity);
                }
                return OPERATOR_FINISHED;
            } else if (event.key.key == SDLK_D && Logic_entt().valid(entity)) {
                if (auto position = Logic_entt().try_get<Transform>(entity)) {
                    position->add_offset({1, 0, 0});
                    Logic_entt().emplace_or_replace<Camera_dirty>(entity);
                }
                return OPERATOR_FINISHED;
            } else if (event.key.key == SDLK_X && Logic_entt().valid(entity)) {
                if (Logic_entt().valid(entity)) {
                    Logic_entt().emplace_or_replace<Logic_destroy_tag>(entity);
                    return OPERATOR_FINISHED;
                }
                return OPERATOR_FINISHED;
            } else if (event.key.key == SDLK_SPACE && Logic_entt().valid(entity)) {
                if (auto position = Logic_entt().try_get<Transform>(entity)) {
                    if (event.key.mod & SDL_KMOD_SHIFT)
                        position->add_offset({0, -1, 0});
                    else
                        position->add_offset({0, 1, 0});
                    Logic_entt().emplace_or_replace<Camera_dirty>(entity);
                }
                return OPERATOR_FINISHED;
            } else if (event.key.key == SDLK_ESCAPE && Logic_entt().valid(entity)) {
                return OPERATOR_CANCELLED;
            } else {
                return OPERATOR_PASS_THROUGH;
            }
        }
        case SDL_EVENT_FINGER_MOTION: {
            break;
        }
        case SDL_EVENT_WINDOW_MOUSE_ENTER: {
            break;
        }
        case SDL_EVENT_WINDOW_MOUSE_LEAVE: {
            break;
        }
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
        case SDL_EVENT_WINDOW_FOCUS_LOST: {
            break;
        }
        case SDL_EVENT_GAMEPAD_ADDED:
        case SDL_EVENT_GAMEPAD_REMOVED: {
            break;
        }

        case SDL_EVENT_MOUSE_MOTION: {
            if (status.select_status_ == select_current) {
                // TODO : 没有确定坐标或者说坐标的系数
                Eigen::Vector2f current_position{event.motion.x, event.motion.y};
                Eigen::Vector2f last_position{
                    event.motion.x - event.motion.xrel,
                    event.motion.y - event.motion.yrel
                };

                if (auto *transform = Logic_entt().try_get<Transform>(entity)) {
                    const auto object_position = transform->get_offset();
                    const auto world_entity    = get_world_root();
                    const auto current_ray     = get_screen_ray(current_position);
                    const auto last_ray        = get_screen_ray(last_position);

                    const auto camera_position = Logic_entt().try_get<Transform>(world_entity);

                    const auto quat        = camera_position->get_rotate();
                    Eigen::Vector3f normal = quat * Eigen::Vector3f::UnitZ(); // 假设法向量指向 Z 轴
                    normal.normalize();                                       // 这个法线的求法是对的吗？
                    // normal 其实是 view direction
                    // Eigen::Vector3f normal = camera_matrix.block<3, 1>(0, 2); // 另一种拿 法线的办法
                    // quat 乘于 unit Z (0,0,1) 的结果是可以被简化的 ，之后再看
                    const Plane<Eigen::Vector3f> plane{object_position, normal};
                    // 只剩 下面这一个问题了
                    const auto a = intersect_result(plane, current_ray);
                    const auto b = intersect_result(plane, last_ray);

                    transform->add_offset({(a - b).x(), (a - b).y(), (a - b).z()});
                    // 这里 y 需要乘与一个 负号的 原因是因为 拿到的 屏幕的坐标 与 归一化坐标不一致
                    Logic_entt().emplace_or_replace<UI_transform_dirty>(entity);
                    return OPERATOR_RUNNING_MODAL;
                }
            }
            return OPERATOR_PASS_THROUGH;
        }
        default:
            break;
    }

    return
            OPERATOR_PASS_THROUGH;
}

void update_camera_parameter(const entt::entity entity) {
    auto camera                           = Logic_entt().get_or_emplace<camera_optical_component>(entity);
    const auto projection                 = camera.get_projection_matrix();
    Eigen::Matrix4f inv_projection_matrix = projection.inverse();
    // const Point_3 world_light_pos{0, 10, 6};

    const auto view_matrix           = camera.get_view_matrix();
    Eigen::Vector3f world_camera_pos = camera.get_position();
    Eigen::Matrix4f inv_view_matrix  = view_matrix.inverse();

    Eigen::Matrix4f invVP   = (projection * view_matrix).inverse();
    Eigen::Matrix4f invVP_3 = inv_view_matrix * inv_projection_matrix;

    Engine::instance().set_projection_matrix(projection);
    Engine::instance().set_inv_projection_matrix(inv_projection_matrix);
    Engine::instance().set_view_matrix(view_matrix);
    Engine::instance().set_inv_view_matrix(inv_view_matrix);
    Engine::instance().set_world_camera_pos(world_camera_pos);
    Engine::instance().set_invVP(invVP);


    // Engine::instance().set_sun_light({world_light_pos.x, world_light_pos.y, world_light_pos.z});
}


void init_world_scene_root(entt::entity entity) {
    Logic_entt().emplace<Scene_Component>(entity);
    Logic_entt().emplace<Name_component>(entity, "world_scene_root");
    Logic_entt().emplace<Move_speed>(entity);

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

    // allocate_descriptor_sets(entity, "bindless"); // todo : 需要确定放在哪里？
}


void update_camera_transform() {
    const auto view = Logic_entt().view<Camera_dirty, Name_component, camera_optical_component>();
    for (const auto it: view) {
        auto &camera = view.get<camera_optical_component>(it);
        auto &name   = view.get<Name_component>(it);
        if (name.name_.find("world_scene_root") != std::string::npos) {
            update_camera_parameter(it);
            auto lambda = [](const entt::entity entity) {
                if (Logic_entt().all_of<Scene_Component>(entity))
                    Logic_entt().emplace_or_replace<UI_transform_dirty>(entity);
            };
            add_recursion_function_to_children(it, lambda);
        }
        Logic_entt().remove<Camera_dirty>(it);
    }
}
