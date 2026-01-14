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

// 组合键状态管理器（仅负责按键状态+组合键匹配）
class Keyboard_Manage {
public:
    using ComboRule = std::unordered_set<int>; // 组合键规则（GLFW按键码集合）


    // 构造函数：依赖注入事件队列管理器
    Keyboard_Manage() = default;

    bool init_eventQueueMgr(queue_thread_safe<base_event_with_stamp> *eventQueueMgr, entt::dispatcher *dispatcher) {
        ptr_event_queue = eventQueueMgr;
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

    // 注册组合键规则（线程安全）
    void register_key_combination(const std::string &key_combination_name) {
        // std::vector<std::string> parts = splitKeyString(testCase);
        auto temp = parseGLFWKeyString(key_combination_name);

        std::lock_guard<std::mutex> lock(_mutex);
        const ComboRule rule = temp;
        key_combination_rules[key_combination_name] = rule;
    }

    // 注销组合键规则（线程安全）
    void unregister_key_combination(const std::string &key_combination_name) {
        std::lock_guard<std::mutex> lock(_mutex);
        key_combination_rules.erase(key_combination_name);
    }

    // 处理GLFW按键按下事件（更新Set状态）
    void handleKeyDown(int keyCode) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (keyCode == GLFW_KEY_UNKNOWN) return;
        if (!pressed_keys.count(keyCode)) {
            // 不想重复处理一个已经按下的键
            pressed_keys.insert(keyCode); // Set更新按下状态

            match_combination(); // 匹配组合键
        } else {
            // 其实还可以再加另一个时间戳，
            // 判断时间，然后超过一定时间之后，再更新时间戳，并进行按键的匹配操作
        }
    }

    void handle_mouse_click_left(mouse_position pos) {
        pos_mouse_button_left_click = pos;
        first_pos_mouse_left_click = pos;
        mouse_button_left_click = true;
        ptr_event_queue->push({EventType::mouse_click_left, "mouse_click_left", pos});
        dispatcher_->enqueue<base_event_with_stamp>({EventType::mouse_click_left, "mouse_click_left", pos});
    }

    void handle_mouse_click_right(mouse_position pos) {
        pos_mouse_button_right_click = pos;
        first_pos_mouse_right_click = pos;
        mouse_button_right_click = true;
        ptr_event_queue->push({EventType::mouse_click_right, "mouse_click_right", pos});
        dispatcher_->enqueue<base_event_with_stamp>({EventType::mouse_click_right, "mouse_click_right", pos});
    }


    void handle_drag(mouse_position pos) {
        if (mouse_button_left_click == true && !(pos == pos_mouse_button_left_click)) {
            const auto error = pos - pos_mouse_button_left_click;
            ptr_event_queue->push({
                EventType::drag, "mouse_button_left_drag",
                base_event_with_stamp::Drag{pos, error}
            });
            dispatcher_->enqueue<base_event_with_stamp>({
                EventType::drag, "mouse_button_left_drag",
                base_event_with_stamp::Drag{pos, error}
            });
            // 检查 被选中的物品
            // 检查 pos_mouse_button_left_click 是否已经在已经选择的物品了，是可以直接移动物品，否的话见下最后一行注释
            // 那么就是下面的一个问题，物体的 pos_mouse_button_left_click 是否还需要更新呢？
            pos_mouse_button_left_click = pos;
            // 更新是可以的，因为物体的移动是伴随 pos_mouse_button_left_click 移动的，
            // 移动的事件是同步的

            // 未选中时也需要处理这个事件，因为有一个选择框需要显示
        } else if (mouse_button_right_click == true && !(pos == pos_mouse_button_right_click)) {
            const auto error = pos - pos_mouse_button_right_click;
            ptr_event_queue->push({
                EventType::drag, "mouse_button_right_drag",
                base_event_with_stamp::Drag{pos, error}
            });
            dispatcher_->enqueue<base_event_with_stamp>({
                EventType::drag, "mouse_button_right_drag",
                base_event_with_stamp::Drag{pos, error}
            });
            pos_mouse_button_right_click = pos;
        }
    }

    void handle_scroll(mouse_position pos) {
        ptr_event_queue->push({EventType::scroll, "mouse_scroll_zoom", pos});
        dispatcher_->enqueue<base_event_with_stamp>(EventType::scroll, "mouse_scroll_zoom", pos);
    }

    void handle_mouse_release_left(const mouse_position release_pos) {
        mouse_button_left_click = false;
        if (abs(release_pos - first_pos_mouse_left_click) < error_between_click_and_release) {
            ptr_event_queue->push({
                EventType::mouse_release_left, "mouse_release_left",
                {first_pos_mouse_left_click, release_pos}
            });
            dispatcher_->enqueue<base_event_with_stamp>({
                EventType::mouse_release_left, "mouse_release_left",
                {first_pos_mouse_left_click, release_pos}
            });
        } else {
            {
                ptr_event_queue->push({
                    EventType::area_select, "left_area_select",
                    {first_pos_mouse_left_click, release_pos}
                });
                dispatcher_->enqueue<base_event_with_stamp>({
                    EventType::area_select, "left_area_select",
                    {first_pos_mouse_left_click, release_pos}
                });
            }
        }
    }

    // 选择与拖动的区别，如果已经在已经选择的物品了，那么可以直接移动物品
    // 如果在首次的坐标不再选择的物品上，那么直接走到选择的逻辑
    // 如果拖着事件已经中了，那么如果处理选择框的事件呢？
    // 如果鼠标按键按下到松开的时间内有拖拽事件处理成功，那么丢弃掉这个事件

    void handle_mouse_release_right(const mouse_position release_pos) {
        mouse_button_right_click = false;
        if (abs(release_pos - first_pos_mouse_right_click) < error_between_click_and_release)
        // dispatcher.enqueue<KeyEvent>(27, true);
        {
            ptr_event_queue->push({
                EventType::mouse_release_right, "mouse_release_right",
                {first_pos_mouse_right_click, release_pos}
            });
            dispatcher_->enqueue<base_event_with_stamp>({
                EventType::mouse_release_right, "mouse_release_right",
                {first_pos_mouse_right_click, release_pos}
            });
        } else {
            // 最终松开时才会处理的选择的逻辑
            ptr_event_queue->push({
                EventType::area_select, "right_area_select",
                {first_pos_mouse_right_click, release_pos}
            });
            dispatcher_->enqueue<base_event_with_stamp>({
                EventType::area_select, "right_area_select",
                {first_pos_mouse_right_click, release_pos}
            });
        }
    }

    // 处理GLFW按键松开事件（更新Set状态）
    void handleKeyUp(int keyCode) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (keyCode == GLFW_KEY_UNKNOWN) return;
        pressed_keys.erase(keyCode); // Set移除松开状态
    }

    // 清空所有状态（窗口失焦时）
    void clearState() {
        std::lock_guard<std::mutex> lock(_mutex);
        pressed_keys.clear();
    }

    static Keyboard_Manage &instance() {
        static auto *instance = new Keyboard_Manage();
        return *instance;
    }

private:
    bool mouse_button_left_click = false;
    bool mouse_button_right_click = false;
    mouse_position pos_mouse_button_left_click;
    mouse_position pos_mouse_button_right_click;
    mouse_position first_pos_mouse_left_click;
    mouse_position first_pos_mouse_right_click;
    mouse_position error_between_click_and_release = {5, 5};          // 这里的范围有问题，需要更改
    std::mutex _mutex;                                                // 线程安全锁
    std::unordered_set<int> pressed_keys;                             // Set：当前按下的按键
    std::unordered_map<std::string, ComboRule> key_combination_rules; // 组合键规则映射
    queue_thread_safe<base_event_with_stamp> *ptr_event_queue;        // 依赖注入的事件队列管理器

    entt::dispatcher *dispatcher_;

    // 组合键匹配：匹配成功则委托事件队列入队（内部辅助）
    void match_combination() {
        double now = glfwGetTime(); // GLFW时间戳
        for (const auto &key: key_combination_rules) {
            auto key_combination_name = key.first;
            auto unordered_set_keys = key.second;
            // 检查规则中所有按键是否都在Set中
            bool exactMatch = std::all_of(unordered_set_keys.begin(), unordered_set_keys.end(),
                                          [&](int k) { return pressed_keys.count(k); })
                              && std::all_of(pressed_keys.begin(), pressed_keys.end(),
                                             [&](int k) { return unordered_set_keys.count(k); });

            // 匹配成功且未触发过 → 委托事件队列入队
            if (exactMatch) {
                ptr_event_queue->push({EventType::key_combination, key_combination_name});
                dispatcher_->enqueue<base_event_with_stamp>({EventType::key_combination, key_combination_name});
            }
        }
    }
};


#endif //LEARN_OPENGL_INPUT_DEVICE_MANAGE_H
