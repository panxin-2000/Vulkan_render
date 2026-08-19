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


class Combined_shortcut_keys {
    std::unordered_set<uint16_t> keys_;
    std::bitset<512> new_key_;


    // if ((current_input & required_combo) == required_combo) {
    //     // 快捷键组合触发成功（哪怕玩家同时按了别的没用的键）
    // }


public:
    void add_pressed_key(const SDL_Scancode code) {
        new_key_[code] = true;
    }

    static std::string toLower(const std::string &str) {
        std::string lowerStr;
        for (const char ch: str) {
            lowerStr += std::tolower(static_cast<unsigned char>(ch));
        }
        return lowerStr;
    }


    std::map<std::string, uint16_t> buildKeyMap() {
        std::map<std::string, uint16_t> keyMap;

        // ==========================================
        // 1. 修饰键 (Modifiers)
        // ==========================================
        keyMap["shift"]  = SDL_SCANCODE_LSHIFT;
        keyMap["lshift"] = SDL_SCANCODE_LSHIFT;
        keyMap["rshift"] = SDL_SCANCODE_RSHIFT;
        keyMap["ctrl"]   = SDL_SCANCODE_LCTRL;
        keyMap["lctrl"]  = SDL_SCANCODE_LCTRL;
        keyMap["rctrl"]  = SDL_SCANCODE_RCTRL;
        keyMap["alt"]    = SDL_SCANCODE_LALT;
        keyMap["lalt"]   = SDL_SCANCODE_LALT;
        keyMap["ralt"]   = SDL_SCANCODE_RALT;
        keyMap["win"]    = SDL_SCANCODE_LGUI;
        keyMap["lgui"]   = SDL_SCANCODE_LGUI;
        keyMap["rgui"]   = SDL_SCANCODE_RGUI;

        // ==========================================
        // 2. 字母键 (A-Z)
        // ==========================================
        keyMap["a"] = SDL_SCANCODE_A;
        keyMap["b"] = SDL_SCANCODE_B;
        keyMap["c"] = SDL_SCANCODE_C;
        keyMap["d"] = SDL_SCANCODE_D;
        keyMap["e"] = SDL_SCANCODE_E;
        keyMap["f"] = SDL_SCANCODE_F;
        keyMap["g"] = SDL_SCANCODE_G;
        keyMap["h"] = SDL_SCANCODE_H;
        keyMap["i"] = SDL_SCANCODE_I;
        keyMap["j"] = SDL_SCANCODE_J;
        keyMap["k"] = SDL_SCANCODE_K;
        keyMap["l"] = SDL_SCANCODE_L;
        keyMap["m"] = SDL_SCANCODE_M;
        keyMap["n"] = SDL_SCANCODE_N;
        keyMap["o"] = SDL_SCANCODE_O;
        keyMap["p"] = SDL_SCANCODE_P;
        keyMap["q"] = SDL_SCANCODE_Q;
        keyMap["r"] = SDL_SCANCODE_R;
        keyMap["s"] = SDL_SCANCODE_S;
        keyMap["t"] = SDL_SCANCODE_T;
        keyMap["u"] = SDL_SCANCODE_U;
        keyMap["v"] = SDL_SCANCODE_V;
        keyMap["w"] = SDL_SCANCODE_W;
        keyMap["x"] = SDL_SCANCODE_X;
        keyMap["y"] = SDL_SCANCODE_Y;
        keyMap["z"] = SDL_SCANCODE_Z;

        // ==========================================
        // 3. 数字键 (主键盘区 0-9)
        // ==========================================
        keyMap["1"] = SDL_SCANCODE_1;
        keyMap["2"] = SDL_SCANCODE_2;
        keyMap["3"] = SDL_SCANCODE_3;
        keyMap["4"] = SDL_SCANCODE_4;
        keyMap["5"] = SDL_SCANCODE_5;
        keyMap["6"] = SDL_SCANCODE_6;
        keyMap["7"] = SDL_SCANCODE_7;
        keyMap["8"] = SDL_SCANCODE_8;
        keyMap["9"] = SDL_SCANCODE_9;
        keyMap["0"] = SDL_SCANCODE_0;

        // ==========================================
        // 4. 功能键 (F1-F12)
        // ==========================================
        keyMap["f1"]  = SDL_SCANCODE_F1;
        keyMap["f2"]  = SDL_SCANCODE_F2;
        keyMap["f3"]  = SDL_SCANCODE_F3;
        keyMap["f4"]  = SDL_SCANCODE_F4;
        keyMap["f5"]  = SDL_SCANCODE_F5;
        keyMap["f6"]  = SDL_SCANCODE_F6;
        keyMap["f7"]  = SDL_SCANCODE_F7;
        keyMap["f8"]  = SDL_SCANCODE_F8;
        keyMap["f9"]  = SDL_SCANCODE_F9;
        keyMap["f10"] = SDL_SCANCODE_F10;
        keyMap["f11"] = SDL_SCANCODE_F11;
        keyMap["f12"] = SDL_SCANCODE_F12;

        // ==========================================
        // 5. 控制与动作键 (Control & Action Keys)
        // ==========================================
        keyMap["return"]    = SDL_SCANCODE_RETURN;
        keyMap["enter"]     = SDL_SCANCODE_RETURN; // 常用别名
        keyMap["escape"]    = SDL_SCANCODE_ESCAPE;
        keyMap["esc"]       = SDL_SCANCODE_ESCAPE; // 常用别名
        keyMap["backspace"] = SDL_SCANCODE_BACKSPACE;
        keyMap["tab"]       = SDL_SCANCODE_TAB;
        keyMap["space"]     = SDL_SCANCODE_SPACE;
        keyMap["capslock"]  = SDL_SCANCODE_CAPSLOCK;

        // ==========================================
        // 6. 导航与编辑键 (Navigation & Edit Keys)
        // ==========================================
        keyMap["printscreen"] = SDL_SCANCODE_PRINTSCREEN;
        keyMap["scrolllock"]  = SDL_SCANCODE_SCROLLLOCK;
        keyMap["pause"]       = SDL_SCANCODE_PAUSE;
        keyMap["insert"]      = SDL_SCANCODE_INSERT;
        keyMap["home"]        = SDL_SCANCODE_HOME;
        keyMap["pageup"]      = SDL_SCANCODE_PAGEUP;
        keyMap["delete"]      = SDL_SCANCODE_DELETE;
        keyMap["end"]         = SDL_SCANCODE_END;
        keyMap["pagedown"]    = SDL_SCANCODE_PAGEDOWN;

        // ==========================================
        // 7. 方向键 (Arrow Keys)
        // ==========================================
        keyMap["right"] = SDL_SCANCODE_RIGHT;
        keyMap["left"]  = SDL_SCANCODE_LEFT;
        keyMap["down"]  = SDL_SCANCODE_DOWN;
        keyMap["up"]    = SDL_SCANCODE_UP;

        // ==========================================
        // 8. 标点符号与特殊符号 (Symbols)
        // ==========================================
        keyMap["numlock"]      = SDL_SCANCODE_NUMLOCKCLEAR;
        keyMap["grave"]        = SDL_SCANCODE_GRAVE;        // ` ~ 键
        keyMap["minus"]        = SDL_SCANCODE_MINUS;        // - _ 键
        keyMap["equals"]       = SDL_SCANCODE_EQUALS;       // = + 键
        keyMap["leftbracket"]  = SDL_SCANCODE_LEFTBRACKET;  // [ { 键
        keyMap["rightbracket"] = SDL_SCANCODE_RIGHTBRACKET; // ] } 键
        keyMap["backslash"]    = SDL_SCANCODE_BACKSLASH;    // \ | 键
        keyMap["semicolon"]    = SDL_SCANCODE_SEMICOLON;    // ; : 键
        keyMap["apostrophe"]   = SDL_SCANCODE_APOSTROPHE;   // ' " 键
        keyMap["comma"]        = SDL_SCANCODE_COMMA;        // , < 键
        keyMap["period"]       = SDL_SCANCODE_PERIOD;       // . > 键
        keyMap["slash"]        = SDL_SCANCODE_SLASH;        // / ? 键

        // ==========================================
        // 9. 数字小键盘区 (Keypad)
        // ==========================================
        keyMap["kp_divide"]   = SDL_SCANCODE_KP_DIVIDE;
        keyMap["kp_multiply"] = SDL_SCANCODE_KP_MULTIPLY;
        keyMap["kp_minus"]    = SDL_SCANCODE_KP_MINUS;
        keyMap["kp_plus"]     = SDL_SCANCODE_KP_PLUS;
        keyMap["kp_enter"]    = SDL_SCANCODE_KP_ENTER;
        keyMap["kp_1"]        = SDL_SCANCODE_KP_1;
        keyMap["kp_2"]        = SDL_SCANCODE_KP_2;
        keyMap["kp_3"]        = SDL_SCANCODE_KP_3;
        keyMap["kp_4"]        = SDL_SCANCODE_KP_4;
        keyMap["kp_5"]        = SDL_SCANCODE_KP_5;
        keyMap["kp_6"]        = SDL_SCANCODE_KP_6;
        keyMap["kp_7"]        = SDL_SCANCODE_KP_7;
        keyMap["kp_8"]        = SDL_SCANCODE_KP_8;
        keyMap["kp_9"]        = SDL_SCANCODE_KP_9;
        keyMap["kp_0"]        = SDL_SCANCODE_KP_0;
        keyMap["kp_period"]   = SDL_SCANCODE_KP_PERIOD;
        keyMap["kp_equals"]   = SDL_SCANCODE_KP_EQUALS;

        return keyMap;
    }

    /**
     * 输入： "'Ctrl' + 'Alt' + 'a'"
     * 输出： {"ctrl", "alt", "a"}（直接过滤掉了其中的 + 号、空格以及外部字符）。
     * @param keyStr
     * @return
     */
    static std::vector<std::string> splitKeyString(const std::string &keyStr) {
        std::vector<std::string> key_strings;
        std::string currentContent; // 存储当前单引号内的内容
        bool inQuote = false;       // 是否进入单引号范围

        for (const char ch: keyStr) {
            // 修复：通过编码判断单引号（半角'=0x27，全角’=0x2019）
            // 注：char 是1字节，全角字符需用 unsigned char 或 wchar_t，此处简化为兼容处理
            bool isQuote = (static_cast<unsigned char>(ch) == 0x27) || // 半角单引号 '
                           (static_cast<unsigned char>(ch) == 0xE2 &&
                            static_cast<unsigned char>(keyStr[keyStr.find(ch) + 1]) == 0x80 &&
                            static_cast<unsigned char>(keyStr[keyStr.find(ch) + 2]) == 0x99); // 全角单引号 ’ 的UTF-8编码

            // 简化版（推荐）：仅处理半角单引号（避免多字节字符判断）
            // bool isQuote = (ch == '\'');

            if (isQuote) {
                // 遇到单引号：切换状态 + 处理已提取的内容
                if (inQuote) {
                    // 退出单引号：清理内容并保存（非空才保存）
                    currentContent.erase(0, currentContent.find_first_not_of(" \t")); // 去前导空格
                    currentContent.erase(currentContent.find_last_not_of(" \t") + 1); // 去尾随空格
                    if (!currentContent.empty()) {
                        key_strings.push_back(toLower(currentContent));
                    }
                    currentContent.clear(); // 重置当前内容
                }
                inQuote = !inQuote; // 切换单引号状态（进入/退出）
            } else if (inQuote) {
                // 处于单引号内：累加字符（包括 + 号）
                currentContent += ch;
            }
            // 非单引号且不在引号内：直接忽略
        }
        // 处理字符串结束时仍未闭合的单引号（残缺单引号）
        if (inQuote && !currentContent.empty()) {
            currentContent.erase(0, currentContent.find_first_not_of(" \t"));
            currentContent.erase(currentContent.find_last_not_of(" \t") + 1);
            if (!currentContent.empty()) {
                key_strings.push_back(toLower(currentContent));
            }
        }
        return key_strings;
    }

    std::unordered_set<uint16_t> parseGLFWKeyString(const std::string &keyStr) {
        static auto keyMap = buildKeyMap();
        std::unordered_set<uint16_t> keys;

        const std::vector<std::string> keyParts = splitKeyString(keyStr);
        for (const std::string &part: keyParts) {
            auto it = keyMap.find(part);
            if (it != keyMap.end()) {
                keys.insert(it->second);
                new_key_[it->second] = true;
            } else {
                std::cerr << "警告：未知键名 -> " << part << "（原始输入：" << keyStr << "）" << std::endl;
            }
        }

        return keys;
    }


    explicit Combined_shortcut_keys(const std::string &key_name) {
        keys_ = parseGLFWKeyString(key_name);
    }

    explicit Combined_shortcut_keys() {
    }
};


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

    base_event_with_stamp handle_mouse_click_left(std::array<float, 2> pos) {
        first_click_position    = pos;
        mouse_button_left_click = true;
        return {
            MOUSE_LEFT,
            KM_PRESS,
            pos,
            last_position,
            first_click_position,
            manage_scroll,
            manage_modifier_flag
        };
    }

    base_event_with_stamp handle_mouse_click_right(std::array<float, 2> pos) {
        first_click_position     = pos;
        mouse_button_right_click = true;
        return {
            MOUSE_RIGHT,
            KM_PRESS,
            pos,
            last_position,
            first_click_position,
            manage_scroll,
            manage_modifier_flag
        };
    }


    base_event_with_stamp handle_drag(std::array<float, 2> pos) {
        current_position = pos;
        if ((mouse_button_left_click == true || mouse_button_right_click == true)
            && !(pos == last_position)) {
            auto temp     = last_position;
            last_position = pos;
            return {
                MOUSE_MOVE,
                KM_PRESS,
                current_position,
                temp,
                first_click_position,
                manage_scroll,
                manage_modifier_flag
            };
        }
        last_position = pos;
        return {};
    }

    base_event_with_stamp handle_scroll(std::array<float, 2> pos) {
        return {
            MOUSE_ROTATE,
            KM_NOTHING,
            current_position,
            last_position,
            first_click_position,
            pos,
            manage_modifier_flag
        };
    }

    base_event_with_stamp handle_mouse_release_left(const std::array<float, 2> release_pos) {
        mouse_button_left_click = false;
        return {
            MOUSE_LEFT,
            KM_RELEASE,
            release_pos,
            last_position,
            first_click_position,
            manage_scroll,
            manage_modifier_flag
        };
    }

    // 选择与拖动的区别，如果已经在已经选择的物品了，那么可以直接移动物品
    // 如果在首次的坐标不再选择的物品上，那么直接走到选择的逻辑
    // 如果拖着事件已经中了，那么如果处理选择框的事件呢？
    // 如果鼠标按键按下到松开的时间内有拖拽事件处理成功，那么丢弃掉这个事件


    base_event_with_stamp handle_mouse_release_right(const std::array<float, 2> release_pos) {
        mouse_button_right_click = false;
        return {
            MOUSE_RIGHT,
            KM_RELEASE,
            release_pos,
            last_position,
            first_click_position,
            manage_scroll,
            manage_modifier_flag
        };
    }

    base_event_with_stamp check_status() {
        return {};
    }


    [[nodiscard]] bool get_focus() const {
        return focus;
    }

private:
    bool focus                    = true;
    bool mouse_button_left_click  = false;
    bool mouse_button_right_click = false;
    wmEventType manage_event_type;
    wmEventModifierFlag manage_modifier_flag;

    std::array<float, 2> current_position                = {0, 0};
    std::array<float, 2> last_position                   = {0, 0};
    std::array<float, 2> first_click_position            = {0, 0};
    std::array<float, 2> manage_scroll                   = {0, 0};
    std::array<float, 2> error_between_click_and_release = {5, 5}; // 这里的范围有问题，需要更改，当是屏幕像素时，就没有改的必要了
};


#endif //LEARN_OPENGL_INPUT_DEVICE_MANAGE_H
