//
// Created by 潘鑫 on 2025/12/15.
//

#ifndef LEARN_OPENGL_INPUT_DEVICE_MANAGE_H
#define LEARN_OPENGL_INPUT_DEVICE_MANAGE_H
#include <unordered_set>
#include <algorithm>
#include <iostream>
#include <SDL3/SDL_scancode.h>
#include <map>
#include <string>
#include <SDL3/SDL.h>

#include "base_event.h"


inline Combined_shortcut_keys check_key() {
    Combined_shortcut_keys keys;
    int num_keys;
    const bool *key_states = SDL_GetKeyboardState(&num_keys);
    for (int i = 0; i < num_keys; ++i) {
        if (key_states[i]) {
            // 如果该键被按下 (值为 true)
            // 将索引转换为 SDL_Scancode
            const SDL_Scancode scancode = static_cast<SDL_Scancode>(i);
            keys.add_pressed_key(scancode);
        }
    }
    return keys;
}

class Mouse_status {
public:
    Mouse_status() = default;

    ~Mouse_status() = default;

    void handle_mouse_click_left(std::array<float, 2> pos) {
    }

    void handle_mouse_click_right(std::array<float, 2> pos) {
        first_click_position     = pos;
        mouse_button_right_click = true;
    }


    void handle_mouse_release_left(const std::array<float, 2> release_pos) {
        mouse_button_left_click = false;
    }

    void handle_mouse_release_right(const std::array<float, 2> release_pos) {
        mouse_button_right_click = false;
    }

    // 选择与拖动的区别，如果已经在已经选择的物品了，那么可以直接移动物品
    // 如果在首次的坐标不再选择的物品上，那么直接走到选择的逻辑
    // 如果拖着事件已经中了，那么如果处理选择框的事件呢？
    // 如果鼠标按键按下到松开的时间内有拖拽事件处理成功，那么丢弃掉这个事件
    base_event_with_stamp add_scroll(const std::array<float, 2> pos) {
    }

    base_event_with_stamp check_status(const SDL_Event &event) {
        current_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>
                (std::chrono::system_clock::now().time_since_epoch());


        keys_ = check_key();

        int num_keys;
        manage_modifier_flag   = KM_NULL;
        const bool *key_states = SDL_GetKeyboardState(&num_keys);
        if (key_states) {
            if (key_states[SDL_SCANCODE_LSHIFT] || key_states[SDL_SCANCODE_RSHIFT]) {
                manage_modifier_flag |= KM_SHIFT;
            }
            if (key_states[SDL_SCANCODE_LCTRL] || key_states[SDL_SCANCODE_RCTRL]) {
                manage_modifier_flag |= KM_CTRL;
            }
            if (key_states[SDL_SCANCODE_LALT] || key_states[SDL_SCANCODE_RALT]) {
                manage_modifier_flag |= KM_ALT;
            }
            if (key_states[SDL_SCANCODE_LGUI] || key_states[SDL_SCANCODE_RGUI]) {
                manage_modifier_flag |= KM_OS_KEY;
            }
        }
        base_event_with_stamp temp{};
        switch (event.type) {
            case SDL_EVENT_MOUSE_WHEEL: {
                manage_event_type = EVENT_SCROLL;
                if (std::abs(event.wheel.x) > std::abs(event.wheel.y)) {
                    manage_scroll = {-event.wheel.x, 0};
                    temp          = get_result();
                } else {
                    manage_scroll = {0, event.wheel.y};
                    temp          = get_result();
                }
            }
            default:
                break;
        }
        // 查询当前鼠标在窗口内的位置和按键状态
        const SDL_MouseButtonFlags buttons = SDL_GetMouseState(&current_position[0], &current_position[1]);
        if (mouse_button_left_click == true && (buttons & SDL_BUTTON_LMASK) && current_position != last_position) {
            manage_event_type = EVENT_PRESS_DOWN_LEFT;
            manage_event_type = EVENT_PRESS_DOWN_RIGHT;
            temp              = get_result();
        }
        if (mouse_button_right_click == true && (buttons & SDL_BUTTON_RMASK) && current_position != last_position) {
            manage_event_type = EVENT_PRESS_DOWN_RIGHT;
            temp              = get_result();
        }

        if (buttons & SDL_BUTTON_LMASK) {
            // 左键正被按下
            if (mouse_button_left_click == false) {
                manage_event_type       = EVENT_FIRST_LEFT;
                first_click_position    = current_position;
                mouse_button_left_click = true;
                temp                    = get_result();
            }
        } else {
            if (mouse_button_left_click == true) {
                manage_event_type       = EVENT_RELEASE_LEFT;
                mouse_button_left_click = false;
                temp                    = get_result();
            }
        }
        if (buttons & SDL_BUTTON_RMASK) {
            // 右键正被按下
            if (mouse_button_right_click == false) {
                manage_event_type        = EVENT_FIRST_RIGHT;
                first_click_position     = current_position;
                mouse_button_right_click = true;
                temp                     = get_result();
            }
        } else {
            if (mouse_button_right_click == true) {
                manage_event_type        = EVENT_RELEASE_RIGHT;
                mouse_button_right_click = false;
                temp                     = get_result();
            }
        }
        if (manage_event_type != EVENT_NONE) {
            std::cout << " current_timestamp " << current_timestamp << std::endl;
            std::cout << " last_timestamp " << last_timestamp << std::endl;
            last_position     = current_position;
            last_timestamp    = current_timestamp;
            manage_event_type = EVENT_NONE;
            return temp;
        }
        last_position     = current_position;
        last_timestamp    = current_timestamp;
        manage_event_type = EVENT_NONE;
        return {};
    }

    [[nodiscard]] bool get_focus() const {
        return focus;
    }

    base_event_with_stamp get_result() {
        return {
            manage_event_type,
            current_position,
            last_position,
            first_click_position,
            manage_scroll,
            manage_modifier_flag,
            keys_,
            current_timestamp,
            last_timestamp
        };
    }

private:
    bool focus                    = true;
    bool mouse_button_left_click  = false;
    bool mouse_button_right_click = false;
    wmEventType manage_event_type;
    wmEventModifierFlag manage_modifier_flag;
    std::chrono::milliseconds last_timestamp;
    std::chrono::milliseconds current_timestamp;


    std::array<float, 2> current_position     = {0, 0};
    std::array<float, 2> last_position        = {0, 0};
    std::array<float, 2> first_click_position = {0, 0};
    std::array<float, 2> manage_scroll        = {0, 0};

    Combined_shortcut_keys keys_;
};


#endif //LEARN_OPENGL_INPUT_DEVICE_MANAGE_H
