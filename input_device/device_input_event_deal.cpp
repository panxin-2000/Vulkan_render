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


entt::dispatcher dispatcher;


#define begin_time_err \
static int64_t last_timestamp = 0;\
uint64_t time_stamp_err       = 0;\
if (last_timestamp == 0) {\
    last_timestamp = event.key.timestamp;\
    time_stamp_err = 1.0 * 1000.0f * 1000.0f * 1000.0f;\
} else {\
time_stamp_err = event.key.timestamp - last_timestamp ;\
}\
float ms      = time_stamp_err / 1000.0f / 1000.0f / 1000.0f;\
float pos_err = speed * ms;

#define end_time_err \
if (event.type == SDL_EVENT_KEY_DOWN) {\
    last_timestamp = event.key.timestamp;\
} else if (event.type == SDL_EVENT_KEY_UP) { \
    last_timestamp = 0;\
}


wmOperatorStatus view_move_up(const entt::entity entity, std::chrono::milliseconds ms) {
    float speed = 1.0f;
    if (const auto move_speed = Logic_entt().try_get<Move_speed>(entity)) {
        speed = move_speed->speed;
    }
    float pos_err = speed * ms.count();
    std::cout << " std::chrono::milliseconds    " << ms << std::endl;

    if (const auto camera = Logic_entt().try_get<camera_optical_component>(entity)) {
        auto offset = camera->get_view_direction() * -pos_err;
        camera->add_offset(offset);
        Logic_entt().emplace_or_replace<Camera_dirty>(entity);
    }
    return OPERATOR_FINISHED;
}

wmOperatorStatus view_move_down(const entt::entity entity, std::chrono::milliseconds ms) {
    float speed = 1.0f;
    if (const auto move_speed = Logic_entt().try_get<Move_speed>(entity)) {
        speed = move_speed->speed;
    }
    float pos_err = speed * ms.count();
    if (const auto camera = Logic_entt().try_get<camera_optical_component>(entity)) {
        auto offset = camera->get_view_direction() * pos_err;
        camera->add_offset(offset);
        Logic_entt().emplace_or_replace<Camera_dirty>(entity);
    }
    return OPERATOR_FINISHED;
}

wmOperatorStatus view_move_left(const entt::entity entity, std::chrono::milliseconds ms) {
    float speed = 1.0f;
    if (const auto move_speed = Logic_entt().try_get<Move_speed>(entity)) {
        speed = move_speed->speed;
    }
    float pos_err = speed * ms.count();
    if (const auto camera = Logic_entt().try_get<camera_optical_component>(entity)) {
        auto offset = camera->get_view_right_direction() * -pos_err;
        camera->add_offset(offset);
        Logic_entt().emplace_or_replace<Camera_dirty>(entity);
    }
    return OPERATOR_FINISHED;
}

wmOperatorStatus view_move_right(const entt::entity entity, std::chrono::milliseconds ms) {
    float speed = 1.0f;
    if (const auto move_speed = Logic_entt().try_get<Move_speed>(entity)) {
        speed = move_speed->speed;
    }
    float pos_err = speed * ms.count();
    if (const auto camera = Logic_entt().try_get<camera_optical_component>(entity)) {
        auto offset = camera->get_view_right_direction() * pos_err;
        camera->add_offset(offset);
        Logic_entt().emplace_or_replace<Camera_dirty>(entity);
    }
    return OPERATOR_FINISHED;
}


static wmOperatorStatus world_rotate(const entt::entity entity, const Point_2 temp) {
    if (auto camera = Logic_entt().try_get<camera_optical_component>(entity)) {
        auto q_current = camera->get_rotate();
        q_current      = Eigen::Quaternionf(Eigen::AngleAxisf(temp.x / 100, Eigen::Vector3f::UnitY()) *
                                       q_current);
        q_current =
                q_current * Eigen::Quaternionf(Eigen::AngleAxisf(temp.y / 100, Eigen::Vector3f::UnitX()));
        q_current.normalize();
        camera->set_rotate(q_current);
        Logic_entt().emplace_or_replace<Camera_dirty>(entity);
        return OPERATOR_FINISHED;
    } else {
        return OPERATOR_PASS_THROUGH;
    }
}


static wmOperatorStatus world_root_on_Event(const entt::entity entity, const SDL_Event &event,
                                            std::optional<base_event_with_stamp> mouse) {
    if (mouse.has_value()) {
        switch (mouse.value().event_type) {
            case EVENT_NONE: {
                break;
            }
            case EVENT_FIRST_LEFT: {
                break;
            }
            case EVENT_PRESS_DOWN_LEFT: {
                break;
            }
            case EVENT_RELEASE_LEFT: {
                break;
            }
            case EVENT_FIRST_RIGHT: {
                break;
            }
            case EVENT_PRESS_DOWN_RIGHT: {
                break;
            }
            case EVENT_RELEASE_RIGHT: {
                break;
            }
            case EVENT_SCROLL: {
                const Point_2 temp{mouse.value().scroll[0], mouse.value().scroll[1]};
                return world_rotate(entity, temp);
            }
            case EVENT_MOVE: {
                break;
            }
            case EVENT_KEY_DOWN:
            case EVENT_KEY_FIRST_DOWN: {
                Combined_shortcut_keys temp(" 'w' ");
                if (mouse->keys_ == temp) {
                    view_move_up(entity, mouse->key_error_timestamp);
                }
                break;
            }
        }
    }
    return OPERATOR_PASS_THROUGH;
}


#include "base_geometry/intersect_function.h"

entt::entity find_entity_insert_ray(const Ray<Eigen::Vector3f> &ray) {
    // ray.direction   = {0.0001, 0.0001, -1};
    const auto view = Logic_entt().view<Name_component, World_Space_AABB>();
    for (auto &entity: view) {
        auto &name     = view.get<Name_component>(entity);
        auto bound_box = view.get<World_Space_AABB>(entity);

        AABB_min_max<Point_3> new_box = {
            {
                bound_box.min.x(),
                bound_box.min.y(),
                bound_box.min.z()
            },
            {
                bound_box.max.x(),
                bound_box.max.y(),
                bound_box.max.z()
            }
        };
        const Ray<Point_3> ray_temp = {
            {ray.point.x(), ray.point.y(), ray.point.z()},
            {ray.direction.x(), ray.direction.y(), ray.direction.z()}
        };
        if (is_intersect(new_box, ray_temp)) {
            auto &name = view.get<Name_component>(entity);
            LOG_INFO(g_log(), " insert box 3d {} ", name.name_);
            return entity;
        }
    }

    return entt::null;
}


void base_event_dealing(const SDL_Event &event, std::optional<base_event_with_stamp> mouse) {
    const auto view = Logic_entt().view<Name_component, Scene_Component, Input_Component>();

    static entt::entity current_select_entity = get_UI_scene_root();
    static wmOperatorStatus current_status    = OPERATOR_ZERO;
    static Eigen::Vector2f mouse_pos{-1, -1};

    switch (event.type) {
        case SDL_EVENT_MOUSE_MOTION: {
            mouse_pos = {(float) event.motion.x, (float) event.motion.y};
            // 还是需要进行一个 计算 的 变换  什么时候完成归一化呢？ //
            break;
        }
        case SDL_EVENT_MOUSE_WHEEL: {
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP: {
        }
        case SDL_EVENT_TEXT_INPUT: {
        }
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
        }
        case SDL_EVENT_WINDOW_MOUSE_ENTER: {
        }
        case SDL_EVENT_WINDOW_MOUSE_LEAVE: {
        }
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
        case SDL_EVENT_WINDOW_FOCUS_LOST: {
        }
        case SDL_EVENT_GAMEPAD_ADDED:
        case SDL_EVENT_GAMEPAD_REMOVED: {
        }
        default:
            break;
    }


    // 鼠标按下时进入模态，移动时，持续模态，鼠标松开时 完成模态 ，按下 ESC 键时，取消模态（ 取消后按键依旧按下，处理需谨慎）
    // 按下 ESC 键时，取消操作，模态已经在，之后的时间不处理，只等鼠标松开取消模态
    // auto &name = view.get<Name_component>(current_select_entity);
    // std::cout << "last work name: " << name.name << std::endl;
    if (current_status == OPERATOR_RUNNING_MODAL && Logic_entt().valid(current_select_entity))
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

    auto ray = get_screen_ray(mouse_pos);
    // LOG_INFO(g_log(), "ray {}  {}  {}   direction {} {} {}  ", ray.point.x, ray.point.y, ray.point.z,
    //          ray.direction.x, ray.direction.y, ray.direction.z);


    std::vector<entt::entity> UI_stack = UI_stack_intersect(mouse_pos);

    if (const auto insert_entity = find_entity_insert_ray(ray); insert_entity != entt::null) {
        UI_stack.push_back(insert_entity);
    }
    // std::cout << "UI stack size: " << UI_stack.size() << std::endl;
    // for (auto it = UI_stack.rbegin(); it != UI_stack.rend(); ++it) {
    //     auto &name = view.get<Name_component>(*it);
    //     std::cout << "name: " << name.name_ << std::endl;
    // }

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
    // 需要一个状态来确定需要进入3d来处理
    if (current_status == OPERATOR_ZERO) {
        world_root_on_Event(get_world_root(), event, mouse);
    }
}
