//
// Created by 潘鑫 on 2026/3/18.
//

#include "input_component.h"
#include "base_event.h"
#include "name_component.h"
#include "mesh_component.h"
#include "shader_component.h"
#include <Eigen/Eigen>

#include "model_transform_component.h"


static wmOperatorStatus on_Event(const entt::entity entity_, const base_event_with_stamp &event) {
    auto temp_type = event.event_type;
    auto &status   = Logic_entt().get<Input_Component>(entity_);

    switch (temp_type) {
        case EVT_KEY_X:
            // 删除当前鼠标位置的元素
            if (event.event_code == KM_PRESS)
                if (Logic_entt().valid(entity_)) {
                    // if (const auto render = g_entt().try_get<logic_render_data>(entity_)) {
                    //     render->proxy = nullptr;
                    // }
                    // 加上上面的内容就有问题
                    Logic_entt().emplace_or_replace<Logic_destroy_tag>(entity_);
                    return OPERATOR_FINISHED;
                }
            return OPERATOR_PASS_THROUGH;
            break;
        case EVT_KEY_ESCAPE:
            if (event.event_code == KM_PRESS) {
                std::cout << " button  EVT_KEY_ESCAPE KM_RELEASE" << std::endl;
                // 需要增加模态的处理 返回结束模态 先用按下的状态，之后再更改
                return OPERATOR_CANCELLED;
            }
            break;
        case MOUSE_LEFT:
            if (event.event_code == KM_PRESS) {
                status.select_status = select_current;
                std::cout << " button  MOUSE_LEFT KM_PRESS" << std::endl;
                // 需要增加模态的处理 返回锁定模态
                return OPERATOR_RUNNING_MODAL;
            }
            if (event.event_code == KM_RELEASE) {
                std::cout << " button  MOUSE_LEFT KM_RELEASE" << std::endl;
                // 需要增加模态的处理 返回结束模态
                // auto block_entity = UI_button("新按钮", 10, 10, 220, 220);
                status.select_status = no_select_current;
                return OPERATOR_FINISHED;
            }
            break;
        case MOUSE_RIGHT:
            break;
        case WHEEL_UP_MOUSE:
            if (auto *transform = Logic_entt().try_get<model_transform>(entity_)) {
                // UI->set_zoom(entity_, event);
            }
            break;
        case MOUSE_MOVE:
            if (status.select_status == select_current) {
                if (auto *transform = Logic_entt().try_get<model_transform>(entity_)) {
                    const Point_2 move   = event.current_position - event.last_position;
                    const auto &backend  = VK_backend::get();
                    auto [width, height] = backend.get_current_extent();
                    transform->add_offset({move.x / width * 2, -move.y / height * 2, 0});
                    // 这里 y 需要乘与一个 负号的 原因是因为 拿到的 屏幕的坐标 与 归一化坐标不一致
                    Logic_entt().emplace_or_replace<UI_transform_dirty>(entity_);
                    return OPERATOR_RUNNING_MODAL;
                }
            }
            break;
        default:
            return OPERATOR_PASS_THROUGH;
    }
    return OPERATOR_HANDLED;
}


entt::entity object_3d_model(const std::string &name, const std::string &mesh_path, const Point_3 offset,
                             const Eigen::Quaternionf &rotate = Eigen::Quaternionf::Identity()) {
    entt::entity entity_ = Logic_entt().create();
    Logic_entt().emplace<Name_component>(entity_, name);
    Logic_entt().emplace<Input_Component>(entity_, on_Event);


    Logic_entt().emplace<VKR_shader_paths>(entity_,
                                           "/Users/panxin/CLionProjects/hello_mac/render/shader/multiple_render_targets.vert.spv",
                                           "/Users/panxin/CLionProjects/hello_mac/render/shader/multiple_render_targets.frag.spv",
                                           "", "");
    add_geometry_data(entity_, mesh_path);
    auto [vertices, indices] = load_model(mesh_path);
    add_geometry_data(entity_, vertices, indices);
    auto [min, max] = find_min_max_point(vertices);
    auto &AABB      = Logic_entt().get_or_emplace<AABB_centroid<Point_3> >(entity_, AABB_centroid<Point_3>(min, max));

    // 更新物体的模型矩阵
    Logic_entt().emplace<model_transform>(entity_, offset, rotate);
    auto &transform = Logic_entt().get<model_transform>(entity_);

    const auto modelMatrix = transform.update_model_matrix();
    set_render_parameter(entity_, "model_4x4", modelMatrix);

    world_root_add_child(entity_);

    Logic_entt().emplace_or_replace<add_to_render_tag>(entity_);
    return entity_;
}
