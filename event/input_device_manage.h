//
// Created by 潘鑫 on 2025/12/15.
//

#ifndef LEARN_OPENGL_INPUT_DEVICE_MANAGE_H
#define LEARN_OPENGL_INPUT_DEVICE_MANAGE_H
#include <unordered_set>
#include <unordered_map>
#include <mutex>
#include <algorithm>
#include <iostream>
#include <GLFW/glfw3.h>
#include "../input_device/key_map_value.h"
#include <entt/entt.hpp>
#include <SDL3/SDL_scancode.h>

#include "base_event.h"


class Combined_shortcut_keys {
    std::unordered_set<uint16_t> keys_;
    std::bitset<512> new_key_;

public:
    static std::string toLower(const std::string &str) {
        std::string lowerStr;
        for (const char ch: str) {
            lowerStr += std::tolower(static_cast<unsigned char>(ch));
        }
        return lowerStr;
    }


    std::map<std::string, uint16_t> buildKeyMap() {
        std::map<std::string, uint16_t> keyMap;

        // 修饰键
        keyMap["shift"]   = SDL_SCANCODE_LSHIFT;
        keyMap["lshift"]  = SDL_SCANCODE_LSHIFT;
        keyMap["rshift"]  = SDL_SCANCODE_RSHIFT;
        keyMap["ctrl"]    = SDL_SCANCODE_LCTRL;
        keyMap["lctrl"]   = SDL_SCANCODE_LCTRL;
        keyMap["rctrl"]   = SDL_SCANCODE_RCTRL;
        keyMap["control"] = SDL_SCANCODE_RCTRL;
        keyMap["alt"]     = SDL_SCANCODE_LALT;
        keyMap["lalt"]    = SDL_SCANCODE_LALT;
        keyMap["ralt"]    = SDL_SCANCODE_RALT;
        keyMap["super"]   = SDL_SCANCODE_LGUI;
        keyMap["meta"]    = SDL_SCANCODE_LGUI;

        // 字母键
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

        // 数字键 + 符号键
        keyMap["1"]      = SDL_SCANCODE_1;
        keyMap["2"]      = SDL_SCANCODE_2;
        keyMap["3"]      = SDL_SCANCODE_3;
        keyMap["4"]      = SDL_SCANCODE_4;
        keyMap["5"]      = SDL_SCANCODE_5;
        keyMap["6"]      = SDL_SCANCODE_6;
        keyMap["7"]      = SDL_SCANCODE_7;
        keyMap["8"]      = SDL_SCANCODE_8;
        keyMap["9"]      = SDL_SCANCODE_9;
        keyMap["0"]      = SDL_SCANCODE_0;
        keyMap["+"]      = SDL_SCANCODE_KP_PLUS;      // 小键盘 + 号（主流）
        keyMap["plus"]   = SDL_SCANCODE_KP_PLUS;      // 兼容 "plus" 写法
        keyMap["kp_add"] = SDL_SCANCODE_KP_PLUS;      // 原生 GLFW 名称
        keyMap["equal"]  = SDL_SCANCODE_EQUALS;       // 主键盘 =/+ 键（兼容）
        keyMap["="]      = SDL_SCANCODE_EQUALS;       // 主键盘 = 号
        keyMap["."]      = SDL_SCANCODE_PERIOD;       // 句号 . 键（核心新增行）
        keyMap["/"]      = SDL_SCANCODE_SLASH;        // 斜杠 / 键（核心新增行）
        keyMap["["]      = SDL_SCANCODE_LEFTBRACKET;  // 左方括号 [
        keyMap["\\"]     = SDL_SCANCODE_BACKSLASH;    // 反斜杠
        keyMap["]"]      = SDL_SCANCODE_RIGHTBRACKET; // 右方括号 ]
        keyMap["`"]      = SDL_SCANCODE_GRAVE;        // 反引号 `
        keyMap[";"]      = SDL_SCANCODE_SEMICOLON;    // 分号 ;
        keyMap["="]      = SDL_SCANCODE_EQUALS;       // 等号 =
        keyMap["'"]      = SDL_SCANCODE_APOSTROPHE;   // 单引号 ' // 有点奇怪
        keyMap[","]      = SDL_SCANCODE_COMMA;        // 逗号 ,
        keyMap["-"]      = SDL_SCANCODE_MINUS;        // 减号 -

        // 功能键
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

        // 特殊键
        keyMap["space"]     = SDL_SCANCODE_SPACE;
        keyMap["enter"]     = SDL_SCANCODE_RETURN;
        keyMap["esc"]       = SDL_SCANCODE_ESCAPE;
        keyMap["escape"]    = SDL_SCANCODE_ESCAPE;
        keyMap["tab"]       = SDL_SCANCODE_TAB;
        keyMap["backspace"] = SDL_SCANCODE_BACKSPACE;
        keyMap["delete"]    = SDL_SCANCODE_DELETE;
        keyMap["up"]        = SDL_SCANCODE_UP;
        keyMap["down"]      = SDL_SCANCODE_DOWN;
        keyMap["left"]      = SDL_SCANCODE_LEFT;
        keyMap["right"]     = SDL_SCANCODE_RIGHT;

        return keyMap;
    }


    static std::vector<std::string> splitKeyString(const std::string &keyStr) {
        std::vector<std::string> key_strings;
        std::string currentContent; // 存储当前单引号内的内容
        bool inQuote = false;       // 是否进入单引号范围

        for (const char ch: keyStr) {
            // 修复：通过编码判断单引号（半角'=0x27，全角’=0x2019）
            // 注：char 是1字节，全角字符需用 unsigned char 或 wchar_t，此处简化为兼容处理
            bool isQuote = (static_cast<unsigned char>(ch) == 0x27) || // 半角单引号 '
                           (static_cast<unsigned char>(ch) == 0xE2 && static_cast<unsigned char>(keyStr[
                                keyStr.find(ch) + 1]) == 0x80 && static_cast<unsigned char>(keyStr[keyStr.find(ch) + 2])
                            ==
                            0x99); // 全角单引号 ’ 的UTF-8编码

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
};


// 组合键状态管理器（仅负责按键状态+组合键匹配）
class Keyboard_Manage {
public:
    using ComboRule = std::unordered_set<uint16_t>; // 组合键规则（GLFW按键码集合）


    // 构造函数：依赖注入事件队列管理器
    Keyboard_Manage() = default;

    bool init_eventQueueMgr(entt::dispatcher *dispatcher) {
        dispatcher_ = dispatcher;
        return true;
    }


    ~Keyboard_Manage() = default;


    // 转换字符串为小写（统一匹配）
    static std::string toLower(const std::string &str) {
        std::string lowerStr;
        for (const char ch: str) {
            lowerStr += std::tolower(static_cast<unsigned char>(ch));
        }
        return lowerStr;
    }


    void handle_mouse_click_left(std::array<float, 2> pos) {
        manage_click_position   = pos;
        mouse_button_left_click = true;
        dispatcher_->enqueue<base_event_with_stamp>({
                                                        MOUSE_LEFT,
                                                        KM_PRESS,
                                                        pos,
                                                        manage_last_position,
                                                        manage_click_position,
                                                        manage_scroll,
                                                        manage_modifier_flag
                                                    });
    }

    void handle_mouse_click_right(std::array<float, 2> pos) {
        manage_click_position    = pos;
        mouse_button_right_click = true;
        dispatcher_->enqueue<base_event_with_stamp>({
                                                        MOUSE_RIGHT,
                                                        KM_PRESS,
                                                        pos,
                                                        manage_last_position,
                                                        manage_click_position,
                                                        manage_scroll,
                                                        manage_modifier_flag
                                                    });
    }


    void handle_drag(std::array<float, 2> pos) {
        manage_current_position = pos;
        if ((mouse_button_left_click == true || mouse_button_right_click == true)
            && !(pos == manage_last_position)) {
            dispatcher_->enqueue<base_event_with_stamp>({
                                                            MOUSE_MOVE,
                                                            KM_PRESS,
                                                            manage_current_position,
                                                            manage_last_position,
                                                            manage_click_position,
                                                            manage_scroll,
                                                            manage_modifier_flag
                                                        });
        }
        manage_last_position = pos;
    }

    void handle_scroll(std::array<float, 2> pos) {
        dispatcher_->enqueue<base_event_with_stamp>({
                                                        MOUSE_ROTATE,
                                                        KM_NOTHING,
                                                        manage_current_position,
                                                        manage_last_position,
                                                        manage_click_position,
                                                        pos,
                                                        manage_modifier_flag
                                                    });
    }

    void handle_mouse_release_left(const std::array<float, 2> release_pos) {
        mouse_button_left_click = false;
        dispatcher_->enqueue<base_event_with_stamp>({
                                                        MOUSE_LEFT,
                                                        KM_RELEASE,
                                                        release_pos,
                                                        manage_last_position,
                                                        manage_click_position,
                                                        manage_scroll,
                                                        manage_modifier_flag
                                                    });
    }

    // 选择与拖动的区别，如果已经在已经选择的物品了，那么可以直接移动物品
    // 如果在首次的坐标不再选择的物品上，那么直接走到选择的逻辑
    // 如果拖着事件已经中了，那么如果处理选择框的事件呢？
    // 如果鼠标按键按下到松开的时间内有拖拽事件处理成功，那么丢弃掉这个事件


    void handle_mouse_release_right(const std::array<float, 2> release_pos) {
        mouse_button_right_click = false;
        dispatcher_->enqueue<base_event_with_stamp>({
                                                        MOUSE_RIGHT,
                                                        KM_RELEASE,
                                                        release_pos,
                                                        manage_last_position,
                                                        manage_click_position,
                                                        manage_scroll,
                                                        manage_modifier_flag
                                                    });
    }


    // 处理GLFW按键按下事件（更新Set状态）
    void handleKeyDown(int keyCode) {
        if (keyCode == EVT_KEY_LEFT_SHIFT || keyCode == EVT_KEY_RIGHT_SHIFT) {
            manage_modifier_flag = manage_modifier_flag | KM_SHIFT;
            return;
        } else if (keyCode == EVT_KEY_LEFT_CONTROL || keyCode == EVT_KEY_RIGHT_CONTROL) {
            manage_modifier_flag = manage_modifier_flag | KM_CTRL;
            return;
        } else if (keyCode == EVT_KEY_LEFT_ALT || keyCode == EVT_KEY_RIGHT_ALT) {
            manage_modifier_flag = manage_modifier_flag | KM_ALT;
            return;
        } else if (keyCode == EVT_KEY_LEFT_SUPER || keyCode == EVT_KEY_RIGHT_SUPER) {
            manage_modifier_flag = manage_modifier_flag | KM_OSKEY;
            return;
        }


        dispatcher_->enqueue<base_event_with_stamp>({
                                                        static_cast<wmEventType>(keyCode),
                                                        KM_PRESS,
                                                        manage_current_position,
                                                        manage_last_position,
                                                        manage_click_position,
                                                        manage_scroll,
                                                        manage_modifier_flag
                                                    });
    }

    // 处理GLFW按键松开事件（更新Set状态）
    void handleKeyUp(int keyCode) {
        if (keyCode == EVT_KEY_LEFT_SHIFT || keyCode == EVT_KEY_RIGHT_SHIFT) {
            manage_modifier_flag &= (~KM_SHIFT);
            return;
        } else if (keyCode == EVT_KEY_LEFT_CONTROL || keyCode == EVT_KEY_RIGHT_CONTROL) {
            manage_modifier_flag &= (~KM_CTRL);
            return;
        } else if (keyCode == EVT_KEY_LEFT_ALT || keyCode == EVT_KEY_RIGHT_ALT) {
            manage_modifier_flag &= (~KM_ALT);
            return;
        } else if (keyCode == EVT_KEY_LEFT_SUPER || keyCode == EVT_KEY_RIGHT_SUPER) {
            manage_modifier_flag &= (~KM_OSKEY);
            return;
        }
        dispatcher_->enqueue<base_event_with_stamp>({
                                                        static_cast<wmEventType>(keyCode),
                                                        KM_RELEASE,
                                                        manage_current_position,
                                                        manage_last_position,
                                                        manage_click_position,
                                                        manage_scroll,
                                                        manage_modifier_flag
                                                    });
        manage_event_type = EVENT_NONE;
    }

    // 清空所有状态（窗口失焦时）
    void clear_focus() {
        std::lock_guard<std::mutex> lock(_mutex);
        focus = false;
        pressed_keys.clear();
    }

    void set_focus() {
        std::lock_guard<std::mutex> lock(_mutex);
        focus = true;
    }

    [[nodiscard]] bool get_focus() const {
        return focus;
    }


    static Keyboard_Manage &instance() {
        static auto *instance = new Keyboard_Manage();
        return *instance;
    }

private:
    bool focus                    = true;
    bool mouse_button_left_click  = false;
    bool mouse_button_right_click = false;
    wmEventType manage_event_type;
    wmEventModifierFlag manage_modifier_flag;

    std::array<float, 2> manage_current_position         = {0, 0};
    std::array<float, 2> manage_last_position            = {0, 0};
    std::array<float, 2> manage_click_position           = {0, 0};
    std::array<float, 2> manage_scroll                   = {0, 0};
    std::array<float, 2> error_between_click_and_release = {5, 5}; // 这里的范围有问题，需要更改，当是屏幕像素时，就没有改的必要了
    std::mutex _mutex;                                             // 线程安全锁
    std::unordered_set<uint16_t> pressed_keys;                     // Set：当前按下的按键
};


#endif //LEARN_OPENGL_INPUT_DEVICE_MANAGE_H
