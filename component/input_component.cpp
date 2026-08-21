//
// Created by 潘鑫 on 2026/8/21.
//

#include "input_component.h"


wmOperatorStatus Input_Component::on_Event(const entt::entity entity, const SDL_Event &event,
                                           std::optional<base_event_with_stamp> mouse) {
    if (mouse.has_value()) {
        switch (mouse.value().event_type) {
            case EVENT_NONE: {
                break;
            }
            case EVENT_FIRST_LEFT: {
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
                if (scroll != nullptr) {
                    scroll(entity, {mouse.value().scroll[0], mouse.value().scroll[1]});
                }
            }
            case EVENT_PRESS_DOWN_LEFT: {
                const Eigen::Vector2f current_position{event.motion.x, event.motion.y};
                const Eigen::Vector2f last_position{
                    event.motion.x - event.motion.xrel,
                    event.motion.y - event.motion.yrel
                };
                if (mouse_drag != nullptr) {
                    return mouse_drag(entity, current_position, last_position);
                }
                break;
            }
            case EVENT_KEY_DOWN:
                for (const auto &shortcut_key: long_press_keys) {
                    if (mouse->keys_ == shortcut_key.first) {
                        return shortcut_key.second(entity, mouse->key_error_timestamp);
                    }
                }
                break;
            case EVENT_KEY_FIRST_DOWN: {
                for (const auto &shortcut_key: shortcut_keys) {
                    if (mouse->keys_ == shortcut_key.first) {
                        return shortcut_key.second(entity, mouse->key_error_timestamp);
                    }
                }
                break;
            }
            case EVENT_KEY_UP:
                break;
            case EVENT_MOVE:
                break;
        }
    }
    return OPERATOR_PASS_THROUGH;
}
