//
// Created by 潘鑫 on 2025/12/15.
//

#ifndef LEARN_OPENGL_INPUT_DEVICE_MANAGE_H
#define LEARN_OPENGL_INPUT_DEVICE_MANAGE_H
#include <unordered_set>
#include <unordered_map>
#include <mutex>
#include <algorithm>
#include <GLFW/glfw3.h>
#include "../input_device/key_map_value.h"
#include <entt/entt.hpp>

// 组合键状态管理器（仅负责按键状态+组合键匹配）
class Keyboard_Manage {
public:
    using ComboRule = std::unordered_set<int>; // 组合键规则（GLFW按键码集合）


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

    static std::vector<std::string> splitKeyString(const std::string &keyStr) {
        std::vector<std::string> keys;
        std::string currentContent; // 存储当前单引号内的内容
        bool inQuote = false;       // 是否进入单引号范围

        for (char ch: keyStr) {
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
                        keys.push_back(toLower(currentContent));
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
                keys.push_back(toLower(currentContent));
            }
        }

        return keys;
    }


    static std::unordered_set<int> parseGLFWKeyString(const std::string &keyStr) {
        static std::unordered_map<std::string, int> keyMap = buildKeyMap();
        std::unordered_set<int> keys;

        std::vector<std::string> keyParts = splitKeyString(keyStr);
        for (const std::string &part: keyParts) {
            auto it = keyMap.find(part);
            if (it != keyMap.end()) {
                keys.insert(it->second);
            } else {
                std::cerr << "警告：未知键名 -> " << part << "（原始输入：" << keyStr << "）" << std::endl;
            }
        }

        return keys;
    }


    void handle_mouse_click_left(mouse_position pos) {
        manage_click_position = pos;
        mouse_button_left_click = true;
        dispatcher_->enqueue<base_event_with_stamp>({
            MOUSE_LEFT,
            KM_CLICK,
            pos,
            manage_last_position,
            manage_click_position,
            manage_scroll,
            KM_SHIFT
        });
    }

    void handle_mouse_click_right(mouse_position pos) {
        manage_click_position = pos;
        mouse_button_right_click = true;
        dispatcher_->enqueue<base_event_with_stamp>({
            MOUSE_RIGHT,
            KM_CLICK,
            pos,
            manage_last_position,
            manage_click_position,
            manage_scroll,
            KM_SHIFT
        });
    }


    void handle_drag(mouse_position pos) {
        manage_current_position = pos;
        if ((mouse_button_left_click == true || mouse_button_right_click == true)
            && !(pos == manage_last_position)) {
            dispatcher_->enqueue<base_event_with_stamp>({
                MOUSE_MOVE,
                KM_CLICK,
                manage_current_position,
                manage_last_position,
                manage_click_position,
                manage_scroll,
                KM_SHIFT
            });
        }
        manage_last_position = pos;
    }

    void handle_scroll(mouse_position pos) {
        dispatcher_->enqueue<base_event_with_stamp>({
            WHEEL_UP_MOUSE,
            KM_CLICK,
            pos,
            manage_last_position,
            manage_click_position,
            pos,
            KM_SHIFT
        });
    }

    void handle_mouse_release_left(const mouse_position release_pos) {
        mouse_button_left_click = false;
        dispatcher_->enqueue<base_event_with_stamp>({
            MOUSE_LEFT,
            KM_RELEASE,
            release_pos,
            manage_last_position,
            manage_click_position,
            manage_scroll,
            KM_SHIFT
        });
    }

    // 选择与拖动的区别，如果已经在已经选择的物品了，那么可以直接移动物品
    // 如果在首次的坐标不再选择的物品上，那么直接走到选择的逻辑
    // 如果拖着事件已经中了，那么如果处理选择框的事件呢？
    // 如果鼠标按键按下到松开的时间内有拖拽事件处理成功，那么丢弃掉这个事件


    void handle_mouse_release_right(const mouse_position release_pos) {
        mouse_button_right_click = false;
        dispatcher_->enqueue<base_event_with_stamp>({
            MOUSE_RIGHT,
            KM_RELEASE,
            release_pos,
            manage_last_position,
            manage_click_position,
            manage_scroll,
            KM_SHIFT
        });
    }


    // 处理GLFW按键按下事件（更新Set状态）
    void handleKeyDown(int keyCode) {
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
    bool focus = true;
    bool mouse_button_left_click = false;
    bool mouse_button_right_click = false;
    wmEventType manage_event_type;
    wmEventModifierFlag manage_modifier_flag;

    mouse_position manage_current_position = {0, 0};
    mouse_position manage_last_position = {0, 0};
    mouse_position manage_click_position = {0, 0};
    mouse_position manage_scroll = {0, 0};
    mouse_position error_between_click_and_release = {5, 5}; // 这里的范围有问题，需要更改，当是屏幕像素时，就没有改的必要了
    std::mutex _mutex;                                       // 线程安全锁
    std::unordered_set<int> pressed_keys;                    // Set：当前按下的按键

    entt::dispatcher *dispatcher_;
};


#endif //LEARN_OPENGL_INPUT_DEVICE_MANAGE_H
