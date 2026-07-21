
#include "transform_component.h"
#include "../event/base_event.h"
#include "name_component.h"
#include "global_singleton.h"
#include "input_component.h"
#include "../event/input_device_manage.h"
#include "scene_component.h"
#include "Rect_2D_component.h"
#include "UI_manager.h"
#include "base_geometry/base.h"


entt::dispatcher dispatcher;

void base_event_dealing(const base_event_with_stamp &event);






static wmOperatorStatus world_root_on_Event(const entt::entity entity, const base_event_with_stamp &event) {
    auto temp_type = event.event_type;

    switch (temp_type) {
        case MOUSE_ROTATE: {
            auto temp = event.scroll;

            if (auto position = Logic_entt().try_get<Transform>(entity)) {
                auto q_current = position->get_rotate();
                q_current = Eigen::Quaternionf(Eigen::AngleAxisf(temp.x / 100, Eigen::Vector3f::UnitY()) * q_current);
                q_current = q_current * Eigen::Quaternionf(Eigen::AngleAxisf(temp.y / 100, Eigen::Vector3f::UnitX()));
                position->set_rotate(q_current);
                Logic_entt().emplace_or_replace<Camera_transform_dirty>(entity);
            }
        }
        case EVT_KEY_W:
            // 删除当前鼠标位置的元素
            if (event.event_code == KM_PRESS)
                if (Logic_entt().valid(entity)) {
                    if (auto position = Logic_entt().try_get<Transform>(entity)) {
                        position->add_offset({0, 0, -1});
                        Logic_entt().emplace_or_replace<Camera_transform_dirty>(entity);
                    }
                    return OPERATOR_FINISHED;
                }
            return OPERATOR_PASS_THROUGH;
        case EVT_KEY_S: {
            if (event.event_code == KM_PRESS)
                if (Logic_entt().valid(entity)) {
                    if (auto position = Logic_entt().try_get<Transform>(entity)) {
                        position->add_offset({0, 0, 1});
                        Logic_entt().emplace_or_replace<Camera_transform_dirty>(entity);
                    }
                    return OPERATOR_FINISHED;
                }
            return OPERATOR_PASS_THROUGH;
            break;
        }
        case EVT_KEY_A: {
            if (event.event_code == KM_PRESS)
                if (Logic_entt().valid(entity)) {
                    if (auto position = Logic_entt().try_get<Transform>(entity)) {
                        position->add_offset({-1, 0, 0});
                        Logic_entt().emplace_or_replace<Camera_transform_dirty>(entity);
                    }
                    return OPERATOR_FINISHED;
                }
            return OPERATOR_PASS_THROUGH;
            break;
        }
        case EVT_KEY_D: {
            if (event.event_code == KM_PRESS)
                if (Logic_entt().valid(entity)) {
                    if (auto position = Logic_entt().try_get<Transform>(entity)) {
                        position->add_offset({1, 0, 0});
                        Logic_entt().emplace_or_replace<Camera_transform_dirty>(entity);
                    }
                    return OPERATOR_FINISHED;
                }
            return OPERATOR_PASS_THROUGH;
            break;
        }
        case EVT_KEY_SPACE_KEY: {
            if (event.event_code == KM_PRESS)
                if (Logic_entt().valid(entity)) {
                    if (auto position = Logic_entt().try_get<Transform>(entity)) {
                        if (event.modifier_flag & KM_SHIFT)
                            position->add_offset({0, -1, 0});
                        else
                            position->add_offset({0, 1, 0});
                        Logic_entt().emplace_or_replace<Camera_transform_dirty>(entity);
                    }
                    return OPERATOR_FINISHED;
                }
            return OPERATOR_PASS_THROUGH;
            break;
        }
        default: {
        }
    }
}


#include "base_geometry/intersect_function.h"

entt::entity find_entity_insert_ray(Ray<Point_3> &ray) {
    const auto view = Logic_entt().view<Name_component, AABB_centroid<Point_3>, Transform>();
    for (auto &entity: view) {
        auto position = view.get<Transform>(entity);
        auto box      = view.get<AABB_centroid<Point_3> >(entity);
        box.add_offset(position.get_position());
        if (is_intersect(box, ray)) {
            auto &name = view.get<Name_component>(entity);
            LOG_INFO(g_log(), " insert box 3d {} ", name.name_);
            return entity;
        }
    }

    return entt::null;
}


void base_event_dealing(const base_event_with_stamp &event) {
    const auto view = Logic_entt().view<Name_component, Scene_Component, Input_Component>();


    static entt::entity current_select_entity = get_UI_scene_root();
    const mouse_position current_position     = event.current_position;
    static wmOperatorStatus current_status    = OPERATOR_ZERO;

    // 鼠标按下时进入模态，移动时，持续模态，鼠标松开时 完成模态 ，按下 ESC 键时，取消模态（ 取消后按键依旧按下，处理需谨慎）
    // 按下 ESC 键时，取消操作，模态已经在，之后的时间不处理，只等鼠标松开取消模态
    auto &name = view.get<Name_component>(current_select_entity);
    // std::cout << "last work name: " << name.name << std::endl;
    if (current_status == OPERATOR_RUNNING_MODAL)
        if (const auto input = Logic_entt().try_get<Input_Component>(current_select_entity)) {
            if (input->on_Event != nullptr) {
                auto status = input->on_Event(current_select_entity, event);
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
        }

    // 找到当前区域的一个递归栈

    auto ray = get_screen_ray(event.current_position);
    LOG_INFO(g_log(), "ray {}  {}  {}   direction {} {} {}  ", ray.point.x, ray.point.y, ray.point.z,
             ray.direction.x, ray.direction.y, ray.direction.z);


    std::vector<entt::entity> UI_stack = UI_stack_intersect(current_position);
    // std::cout << "UI stack size: " << UI_stack.size() << std::endl;
    // for (auto it = UI_stack.rbegin(); it != UI_stack.rend(); ++it) {
    //     auto &name = view.get<Name_component>(*it);
    //     std::cout << "name: " << name.name << std::endl;
    // }

    if (const auto insert_entity = find_entity_insert_ray(ray); insert_entity != entt::null) {
        UI_stack.push_back(insert_entity);
    }
    for (auto it = UI_stack.rbegin(); it != UI_stack.rend(); ++it) {
        if (const auto input = Logic_entt().try_get<Input_Component>(*it)) {
            if (input->on_Event != nullptr) {
                auto status = input->on_Event(*it, event);
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
    }
    world_root_on_Event(get_world_root(), event);

    // 需要一个状态来确定需要进入3d来处理
    if (current_status == OPERATOR_ZERO) {
    }
}
