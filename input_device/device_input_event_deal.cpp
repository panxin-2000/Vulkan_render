#include "camera_optical_component.h"
#include "transform_component.h"
#include "../event/base_event.h"
#include "name_component.h"
#include "global_singleton.h"
#include "input_component.h"
#include "load_gltf_model.h"
#include "move_speed.h"
#include "scene_component.h"
#include "Rect_2D_component.h"
#include "UI_manager.h"
#include "world_scene_root.h"
#include "base_geometry/base.h"


#include "base_geometry/intersect_function.h"

// entt::entity find_entity_insert_ray(const Ray<Eigen::Vector3f> &ray) {
//     // ray.direction   = {0.0001, 0.0001, -1};
//     const auto view = Logic_entt().view<Name_component, World_Space_AABB>();
//     for (auto &entity: view) {
//         auto &name     = view.get<Name_component>(entity);
//         auto bound_box = view.get<World_Space_AABB>(entity);
//
//         AABB_min_max<Point_3> new_box = {
//             {
//                 bound_box.min.x(),
//                 bound_box.min.y(),
//                 bound_box.min.z()
//             },
//             {
//                 bound_box.max.x(),
//                 bound_box.max.y(),
//                 bound_box.max.z()
//             }
//         };
//         const Ray<Point_3> ray_temp = {
//             {ray.point.x(), ray.point.y(), ray.point.z()},
//             {ray.direction.x(), ray.direction.y(), ray.direction.z()}
//         };
//         if (is_intersect(new_box, ray_temp)) {
//             auto &name = view.get<Name_component>(entity);
//             // LOG_INFO(g_log(), " insert box 3d {} ", name.name_);
//             return entity;
//         }
//     }
//
//     return entt::null;
// }


void base_event_dealing(const SDL_Event &event, std::optional<base_event_with_stamp> mouse) {
    const auto view = Logic_entt().view<Name_component, Scene_Component, Input_Component>();

    static entt::entity current_select_entity = get_UI_scene_root();
    static wmOperatorStatus current_status    = OPERATOR_ZERO;
    static Eigen::Vector2f mouse_pos          = {mouse.value().current_position[0], mouse.value().current_position[1]};


    // 鼠标按下时进入模态，移动时，持续模态，鼠标松开时 完成模态 ，按下 ESC 键时，取消模态（ 取消后按键依旧按下，处理需谨慎）
    // 按下 ESC 键时，取消操作，模态已经在，之后的时间不处理，只等鼠标松开取消模态
    // auto &name = view.get<Name_component>(current_select_entity);
    if (current_status == OPERATOR_RUNNING_MODAL && Logic_entt().valid(current_select_entity))
        if (const auto input = Logic_entt().try_get<Input_Component>(current_select_entity)) {
            const auto status = input->on_Event(current_select_entity, event, mouse);
            if (OPERATOR_RUNNING_MODAL & status) {
                current_select_entity = current_select_entity; // 目的是更新，但是没有什么意义
                current_status        = OPERATOR_RUNNING_MODAL;
                return;
            } else if (OPERATOR_FINISHED & status) {
                current_status        = OPERATOR_ZERO;
                current_select_entity = get_UI_scene_root();
                return;
            }
            current_status = OPERATOR_ZERO;
        }

    // 找到当前区域的一个递归栈

    auto ray = get_screen_ray(mouse_pos);
    // LOG_INFO(g_log(), "ray {}  {}  {}   direction {} {} {}  ", ray.point.x, ray.point.y, ray.point.z,
    //          ray.direction.x, ray.direction.y, ray.direction.z);


    std::vector<entt::entity> UI_stack = UI_stack_intersect(mouse_pos);

    // if (const auto insert_entity = find_entity_insert_ray(ray); insert_entity != entt::null) {
    // UI_stack.push_back(insert_entity);
    // }
    // std::cout << "UI stack size: " << UI_stack.size() << std::endl;
    // for (auto it = UI_stack.rbegin(); it != UI_stack.rend(); ++it) {
    //     auto &name = view.get<Name_component>(*it);
    //     std::cout << "name: " << name.name_ << std::endl;
    // }

    for (auto it = UI_stack.rbegin(); it != UI_stack.rend(); ++it) {
        if (const auto input = Logic_entt().try_get<Input_Component>(*it)) {
            const auto status = input->on_Event(*it, event, mouse);
            if (OPERATOR_RUNNING_MODAL & status) {
                current_select_entity = *it;
                current_status        = OPERATOR_RUNNING_MODAL;
                break;
            } else if (OPERATOR_PASS_THROUGH & status) {
                continue;
            } else {
                current_status = OPERATOR_ZERO;
                break;
            }
        }
    }
    // 需要一个状态来确定需要进入3d来处理
    if (current_status == OPERATOR_ZERO) {
        if (const auto input = Logic_entt().try_get<Input_Component>(get_world_root())) {
            input->on_Event(get_world_root(), event, mouse);
        }
    }
}
