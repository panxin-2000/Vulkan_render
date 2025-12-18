// //
// // Created by 潘鑫 on 2025/12/15.
// //
// #include <iostream>
// #include <unordered_set>
// #include <functional>
// #include <GLFW/glfw3.h>
//
// #include "event_base.h"
// #include "observer_manage.h"
//
//
// // 组合键管理器（跨平台，与GLFW解耦）
// class KeyComboManager {
// public:
//     using ComboRule = std::unordered_set<int>; // 组合键规则（GLFW按键码）
//
//     // 构造函数注入观察者管理器（依赖注入，解耦）
//     KeyComboManager() = default;
//
//     ~KeyComboManager() = default;
//
//     // 注册组合键规则
//     void registerCombo(const std::string &comboName, const ComboRule &rule) {
//         std::lock_guard<std::mutex> lock(_mutex);
//         _comboRules[comboName] = rule;
//     }
//
//     // 注销组合键规则
//     void unregisterCombo(const std::string &comboName) {
//         std::lock_guard<std::mutex> lock(_mutex);
//         _comboRules.erase(comboName);
//         _triggeredCombos.erase(comboName);
//     }
//
//     // 处理GLFW按键按下事件（更新Set状态）
//     void “handleKeyDown(int keyCode) {
//         std::lock_guard<std::mutex> lock(_mutex);
//         if (keyCode == GLFW_KEY_UNKNOWN) return;
//
//         _pressedKeys.insert(keyCode); // Set更新按下状态
//         _matchCombos(); // 组合键匹配
//     }
//
//     // 处理GLFW按键松开事件（更新Set状态）
//     void handleKeyUp(int keyCode) {
//         std::lock_guard<std::mutex> lock(_mutex);
//         if (keyCode == GLFW_KEY_UNKNOWN) return;
//
//         _pressedKeys.erase(keyCode); // Set移除松开状态
//         _triggeredCombos.clear(); // 重置触发标记
//     }
//
//     // 清空状态（窗口失焦时）
//     void clearState() {
//         std::lock_guard<std::mutex> lock(_mutex);
//         _pressedKeys.clear();
//         _triggeredCombos.clear();
//         // 清空事件队列
//     }
//
// private:
//     std::mutex _mutex; // 线程安全锁
//     std::unordered_set<int> _pressedKeys; // Set：当前按下的按键
//     std::unordered_map<std::string, ComboRule> _comboRules; // 组合键规则
//     std::unordered_set<std::string> _triggeredCombos; // 已触发组合键（防重复）
//
//     // 组合键匹配：匹配成功则推入事件队列
//     void _matchCombos() {
//         double now = glfwGetTime();
//         for (const auto &[comboName, rule]: _comboRules) {
//             // 检查规则中所有按键是否都在Set中
//             bool isMatch = std::all_of(
//                 rule.begin(), rule.end(),
//                 [this](int key) { return _pressedKeys.count(key); }
//             );
//
//             // 匹配成功且未触发过 → 入队
//             if (isMatch && !_triggeredCombos.count(comboName)) {
//                 // _eventQueue.push({EventType::ComboKey, now});
//                 _triggeredCombos.insert(comboName);
//                 std::cout << "[KeyComboManager] 组合键 " << comboName << " 入队" << std::endl;
//             }
//         }
//     }
// };
//
//
// //
// // #include "KeyComboObserver.h"
// // #include <iostream>
// // #include <GLFW/glfw3.h>
// //
// // // 日志观察者：打印组合键事件
// // class LogObserver : public KeyComboObserver {
// // public:
// //     void onComboEvent(const ComboEvent& event) override {
// //         std::cout << "[LogObserver] 触发组合键：" << event.comboName
// //                   << "（时间戳：" << event.timestamp << "）" << std::endl;
// //     }
// // };
// //
// // // 保存观察者：处理Ctrl+S保存
// // class SaveObserver : public KeyComboObserver {
// // public:
// //     void onComboEvent(const ComboEvent& event) override {
// //         if (event.comboName == "Ctrl+S") {
// //             std::cout << "[SaveObserver] 执行保存操作 → 数据已保存" << std::endl;
// //         }
// //     }
// // };
// //
// // // 全屏观察者：处理Alt+Enter全屏切换
// // class FullscreenObserver : public KeyComboObserver {
// // public:
// //     FullscreenObserver(GLFWwindow* window) : _window(window) {}
// //
// //     void onComboEvent(const ComboEvent& event) override {
// //         if (event.comboName == "Alt+Enter") {
// //             static bool isFullscreen = false;
// //             isFullscreen = !isFullscreen;
// //
// //             if (isFullscreen) {
// //                 GLFWmonitor* monitor = glfwGetPrimaryMonitor();
// //                 const GLFWvidmode* mode = glfwGetVideoMode(monitor);
// //                 glfwSetWindowMonitor(_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
// //             } else {
// //                 glfwSetWindowMonitor(_window, nullptr, 100, 100, 800, 600, 60);
// //             }
// //             std::cout << "[FullscreenObserver] 全屏状态：" << (isFullscreen ? "开启" : "关闭") << std::endl;
// //         }
// //     }
// //
// // private:
// //     GLFWwindow* _window;
// // };
// //
// // // 退出观察者：处理Esc关闭窗口
// // class ExitObserver : public KeyComboObserver {
// // public:
// //     ExitObserver(GLFWwindow* window) : _window(window) {}
// //
// //     void onComboEvent(const ComboEvent& event) override {
// //         if (event.comboName == "Esc") {
// //             glfwSetWindowShouldClose(_window, GLFW_TRUE);
// //             std::cout << "[ExitObserver] 触发Esc → 关闭窗口" << std::endl;
// //         }
// //     }
// //
// // private:
// //     GLFWwindow* _window;
// // };
