//
// Created by 潘鑫 on 2025/12/15.
//

#ifndef LEARN_OPENGL_EVENT_BASE_H
#define LEARN_OPENGL_EVENT_BASE_H
#include <string>

#include <SDL3/SDL.h>

#include "utility.h"

// 事件类型枚举（扩展时新增枚举值即可）
enum class EventType : uint32_t {
    mouse_click_left,    // 鼠标点击事件
    mouse_release_left,  // 鼠标点击事件
    mouse_click_right,   // 鼠标点击事件
    mouse_release_right, // 鼠标点击事件
    scroll,              // 鼠标滚动缩放事件
    drag,                // 鼠标拖动事件
    area_select,         // 鼠标点击事件
    // 上面的都是需要AABB包围盒的，问题是我要AABB包围盒放在哪里？
    // 应该放置在
    key_combination, // 组合键事件
    WindowResize,    // 窗口大小变化事件
    EventType_max
};

enum wmEventType : int16_t {
    /* Non-event, for example disabled timer. */
    EVENT_NONE = 0x0000,
    EVENT_FIRST_LEFT,
    EVENT_PRESS_DOWN_LEFT,
    EVENT_RELEASE_LEFT,
    EVENT_FIRST_RIGHT,
    EVENT_PRESS_DOWN_RIGHT,
    EVENT_RELEASE_RIGHT,
    EVENT_SCROLL,
    EVENT_MOVE,
    EVENT_KEY_FIRST_DOWN,
    EVENT_KEY_DOWN,
    EVENT_KEY_UP,
};


enum wmEventModifierFlag : uint8_t {
    KM_NULL  = 0,
    KM_SHIFT = (1 << 0),
    KM_CTRL  = (1 << 1),
    KM_ALT   = (1 << 2),
    /** Use for Windows-Key on MS-Windows, Command-key on macOS and Super on Linux. */
    KM_OS_KEY = (1 << 3),
    KM_HYPER  = (1 << 4),
};

ENABLE_BITWISE_OPERATORS(wmEventModifierFlag)


struct Drag_event {
};


/**
 * Operator type return flags: exec(), invoke() modal(), return values.
 */
enum wmOperatorStatus {
    OPERATOR_ZERO          = 0,
    OPERATOR_RUNNING_MODAL = (1 << 0),
    OPERATOR_CANCELLED     = (1 << 1),
    OPERATOR_FINISHED      = (1 << 2),
    /** Add this flag if the event should pass through. */
    OPERATOR_PASS_THROUGH = (1 << 3),
    /** In case operator got executed outside WM code (like via file-select). */
    OPERATOR_HANDLED = (1 << 4),
    /**
     * Used for operators that act indirectly (eg. popup menu).
     * \note this isn't great design (using operators to trigger UI) avoid where possible.
     */
    OPERATOR_INTERFACE = (1 << 5),
};

ENABLE_BITWISE_OPERATORS(wmOperatorStatus)


class Combined_shortcut_keys {
    std::bitset<512> new_key_;

    // if ((current_input & required_combo) == required_combo) {
    //     // 快捷键组合触发成功（哪怕玩家同时按了别的没用的键）
    // }


public:
    inline void add_pressed_key(const SDL_Scancode code, const bool value = true) {
        new_key_[code] = value;
    }

    bool operator==(const Combined_shortcut_keys &temp) const = default;

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


    void parse_key_string(const std::string &keyStr) {
        const std::vector<std::string> keyParts = splitKeyString(keyStr);
        parse_key_string(keyParts);
    }

    void parse_key_string(const std::vector<std::string> &keyParts) {
        static auto keyMap = buildKeyMap();
        std::unordered_set<uint16_t> keys;
        for (const std::string &part: keyParts) {
            auto it = keyMap.find(part);
            if (it != keyMap.end()) {
                keys.insert(it->second);
                new_key_[it->second] = true;
            } else {
                std::cerr << "警告：未知键名 -> " << std::endl;
                for (const std::string &key: keyParts) {
                    std::cerr << " " << key << " ";
                }
                std::cerr << std::endl;
            }
        }
    }

    void clear() {
        new_key_.reset();
    }

    int new_press_count(const Combined_shortcut_keys last_key) const {
        const std::bitset<512> bits_added = new_key_ & ~last_key.new_key_; // 多的键
        return static_cast<int>(bits_added.count());
    }

    int new_release_count(const Combined_shortcut_keys last_key) const {
        const std::bitset<512> bits_removed = ~new_key_ & last_key.new_key_; // 少的键
        return static_cast<int>(bits_removed.count());
    }


    explicit Combined_shortcut_keys(const std::string &key_name) {
        parse_key_string(key_name);
    }

    explicit Combined_shortcut_keys(const std::vector<std::string> &key_name) {
        parse_key_string(key_name);
    }

    explicit Combined_shortcut_keys() {
    }
};


/**
 * 通用事件基类
 * mouse_position 返回的否是 0~width 和 0~height
 */
class base_event_with_stamp {
public:
    std::chrono::milliseconds current_timestamp{0};   // 事件时间戳（高精度）
    std::chrono::milliseconds last_timestamp{0};      // 事件时间戳（高精度）
    std::chrono::milliseconds key_error_timestamp{0}; // 事件时间戳（高精度）
    wmEventType event_type;
    std::array<float, 2> current_position; // 当前的鼠标位置
    std::array<float, 2> last_position;
    std::array<float, 2> click_position;
    std::array<float, 2> scroll;
    wmEventModifierFlag modifier_flag;
    Combined_shortcut_keys keys_;

    base_event_with_stamp() = default;

    base_event_with_stamp(const wmEventType event_type,
                          const std::array<float, 2> current_position,
                          const std::array<float, 2> move_position,
                          const std::array<float, 2> click_position,
                          const std::array<float, 2> scroll,
                          const wmEventModifierFlag modifier_flag,
                          const Combined_shortcut_keys keys,
                          const std::chrono::milliseconds current,
                          const std::chrono::milliseconds last,
                          const std::chrono::milliseconds key_error
    )
        : event_type(event_type),
          current_position(current_position),
          last_position(move_position),
          click_position(click_position),
          scroll(scroll),
          keys_(keys),
          modifier_flag(modifier_flag),
          current_timestamp(current),
          key_error_timestamp(key_error),
          last_timestamp(last) {
    }

    ~base_event_with_stamp() = default; // 虚析构保证派生类析构
};


#endif //LEARN_OPENGL_EVENT_BASE_H
