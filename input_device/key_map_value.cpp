//
// Created by 潘鑫 on 2025/12/16.
//
#include "key_map_value.h"
#include <GLFW/glfw3.h>


std::unordered_map<std::string, int> buildKeyMap() {
    std::unordered_map<std::string, int> keyMap;

    // 修饰键
    keyMap["shift"] = GLFW_KEY_LEFT_SHIFT;
    keyMap["lshift"] = GLFW_KEY_LEFT_SHIFT;
    keyMap["rshift"] = GLFW_KEY_RIGHT_SHIFT;
    keyMap["ctrl"] = GLFW_KEY_LEFT_CONTROL;
    keyMap["lctrl"] = GLFW_KEY_LEFT_CONTROL;
    keyMap["rctrl"] = GLFW_KEY_RIGHT_CONTROL;
    keyMap["control"] = GLFW_KEY_LEFT_CONTROL;
    keyMap["alt"] = GLFW_KEY_LEFT_ALT;
    keyMap["lalt"] = GLFW_KEY_LEFT_ALT;
    keyMap["ralt"] = GLFW_KEY_RIGHT_ALT;
    keyMap["super"] = GLFW_KEY_LEFT_SUPER;
    keyMap["meta"] = GLFW_KEY_LEFT_SUPER;

    // 字母键
    keyMap["a"] = GLFW_KEY_A;
    keyMap["b"] = GLFW_KEY_B;
    keyMap["c"] = GLFW_KEY_C;
    keyMap["d"] = GLFW_KEY_D;
    keyMap["e"] = GLFW_KEY_E;
    keyMap["f"] = GLFW_KEY_F;
    keyMap["g"] = GLFW_KEY_G;
    keyMap["h"] = GLFW_KEY_H;
    keyMap["i"] = GLFW_KEY_I;
    keyMap["j"] = GLFW_KEY_J;
    keyMap["k"] = GLFW_KEY_K;
    keyMap["l"] = GLFW_KEY_L;
    keyMap["m"] = GLFW_KEY_M;
    keyMap["n"] = GLFW_KEY_N;
    keyMap["o"] = GLFW_KEY_O;
    keyMap["p"] = GLFW_KEY_P;
    keyMap["q"] = GLFW_KEY_Q;
    keyMap["r"] = GLFW_KEY_R;
    keyMap["s"] = GLFW_KEY_S;
    keyMap["t"] = GLFW_KEY_T;
    keyMap["u"] = GLFW_KEY_U;
    keyMap["v"] = GLFW_KEY_V;
    keyMap["w"] = GLFW_KEY_W;
    keyMap["x"] = GLFW_KEY_X;
    keyMap["y"] = GLFW_KEY_Y;
    keyMap["z"] = GLFW_KEY_Z;

    // 数字键 + 符号键
    keyMap["0"] = GLFW_KEY_0;
    keyMap["1"] = GLFW_KEY_1;
    keyMap["2"] = GLFW_KEY_2;
    keyMap["3"] = GLFW_KEY_3;
    keyMap["4"] = GLFW_KEY_4;
    keyMap["5"] = GLFW_KEY_5;
    keyMap["6"] = GLFW_KEY_6;
    keyMap["7"] = GLFW_KEY_7;
    keyMap["8"] = GLFW_KEY_8;
    keyMap["9"] = GLFW_KEY_9;
    keyMap["+"] = GLFW_KEY_KP_ADD; // 小键盘 + 号（主流）
    keyMap["plus"] = GLFW_KEY_KP_ADD; // 兼容 "plus" 写法
    keyMap["kp_add"] = GLFW_KEY_KP_ADD; // 原生 GLFW 名称
    keyMap["equal"] = GLFW_KEY_EQUAL; // 主键盘 =/+ 键（兼容）
    keyMap["="] = GLFW_KEY_EQUAL; // 主键盘 = 号
    keyMap["."] = GLFW_KEY_PERIOD; // 句号 . 键（核心新增行）
    keyMap["/"] = GLFW_KEY_SLASH; // 斜杠 / 键（核心新增行）
    keyMap["["] = GLFW_KEY_LEFT_BRACKET; // 左方括号 [
    keyMap["\\"] = GLFW_KEY_BACKSLASH; // 反斜杠
    keyMap["]"] = GLFW_KEY_RIGHT_BRACKET; // 右方括号 ]
    keyMap["`"] = GLFW_KEY_GRAVE_ACCENT; // 反引号 `
    keyMap[";"] = GLFW_KEY_SEMICOLON; // 分号 ;
    keyMap["="] = GLFW_KEY_EQUAL; // 等号 =
    keyMap["'"] = GLFW_KEY_APOSTROPHE; // 单引号 ' // 有点奇怪
    keyMap[","] = GLFW_KEY_COMMA; // 逗号 ,
    keyMap["-"] = GLFW_KEY_MINUS; // 减号 -
    keyMap["."] = GLFW_KEY_PERIOD; // 句号 .
    keyMap["/"] = GLFW_KEY_SLASH; // 斜杠 /

    // 功能键
    keyMap["f1"] = GLFW_KEY_F1;
    keyMap["f2"] = GLFW_KEY_F2;
    keyMap["f3"] = GLFW_KEY_F3;
    keyMap["f4"] = GLFW_KEY_F4;
    keyMap["f5"] = GLFW_KEY_F5;
    keyMap["f6"] = GLFW_KEY_F6;
    keyMap["f7"] = GLFW_KEY_F7;
    keyMap["f8"] = GLFW_KEY_F8;
    keyMap["f9"] = GLFW_KEY_F9;
    keyMap["f10"] = GLFW_KEY_F10;
    keyMap["f11"] = GLFW_KEY_F11;
    keyMap["f12"] = GLFW_KEY_F12;

    // 特殊键
    keyMap["space"] = GLFW_KEY_SPACE;
    keyMap["enter"] = GLFW_KEY_ENTER;
    keyMap["esc"] = GLFW_KEY_ESCAPE;
    keyMap["escape"] = GLFW_KEY_ESCAPE;
    keyMap["tab"] = GLFW_KEY_TAB;
    keyMap["backspace"] = GLFW_KEY_BACKSPACE;
    keyMap["delete"] = GLFW_KEY_DELETE;
    keyMap["up"] = GLFW_KEY_UP;
    keyMap["down"] = GLFW_KEY_DOWN;
    keyMap["left"] = GLFW_KEY_LEFT;
    keyMap["right"] = GLFW_KEY_RIGHT;

    return keyMap;
}


// 简化版拆分函数（无全角兼容，100%避免编译问题，推荐生产使用）
/*
std::vector<std::string> splitKeyString(const std::string& keyStr) {
    std::vector<std::string> keys;
    std::string currentContent;
    bool inQuote = false;

    for (char ch : keyStr) {
        if (ch == '\'') { // 仅处理半角单引号，无编译问题
            if (inQuote) {
                // 清理空格并保存
                currentContent.erase(0, currentContent.find_first_not_of(" \t"));
                currentContent.erase(currentContent.find_last_not_of(" \t") + 1);
                if (!currentContent.empty()) {
                    keys.push_back(toLower(currentContent));
                }
                currentContent.clear();
            }
            inQuote = !inQuote;
        } else if (inQuote) {
            currentContent += ch;
        }
    }

    // 处理未闭合的单引号
    if (inQuote && !currentContent.empty()) {
        currentContent.erase(0, currentContent.find_first_not_of(" \t"));
        currentContent.erase(currentContent.find_last_not_of(" \t") + 1);
        if (!currentContent.empty()) {
            keys.push_back(toLower(currentContent));
        }
    }

    return keys;
}
*/
// 解析核心函数

// 键值转字符串（调试用）
std::string glfwKeyToString(int key) {
    switch (key) {
        case GLFW_KEY_LEFT_SHIFT: return "GLFW_KEY_LEFT_SHIFT";
        case GLFW_KEY_LEFT_CONTROL: return "GLFW_KEY_LEFT_CONTROL";
        case GLFW_KEY_LEFT_ALT: return "GLFW_KEY_LEFT_ALT";
        case GLFW_KEY_A: return "GLFW_KEY_A";
        case GLFW_KEY_C: return "GLFW_KEY_C";
        case GLFW_KEY_F4: return "GLFW_KEY_F4";
        case GLFW_KEY_5: return "GLFW_KEY_5";
        case GLFW_KEY_KP_ADD: return "GLFW_KEY_KP_ADD";
        case GLFW_KEY_EQUAL: return "GLFW_KEY_EQUAL";
        default: return "UNKNOWN_KEY";
    }
}

// 测试用例（聚焦单引号内容提取）
// int main() {
//     std::vector<std::string> testCases = {
//         "Shift'+'CTRL'+'+'", // 目标用例：提取 '+'、'CTRL'、'+'
//         "'Shift'+'CTRL'+'+'", // 标准格式：提取 Shift、CTRL、+
//         "'Shift'+'CTRL'+','", // 标准格式：提取 Shift、CTRL、，
//         "'+'", // 单独+号：提取 +
//         "'a'", // 单独+号：提取 +
//         "'  Alt  '+'  F4  '", // 带空格的单引号内容：提取 alt、f4
//         "''+'空内容'+'+'", // 空单引号+中文：提取 空内容、+
//     };
//
//     for (const std::string &testCase: testCases) {
//         std::vector<std::string> parts = splitKeyString(testCase);
//         KeyCombo combo = parseGLFWKeyString(testCase);
//
//         std::cout << "原始输入：" << testCase << std::endl;
//         std::cout << "提取的单引号内容：";
//         if (parts.empty()) {
//             std::cout << "无";
//         } else {
//             for (size_t i = 0; i < parts.size(); ++i) {
//                 if (i > 0) std::cout << ", ";
//                 std::cout << "\"" << parts[i] << "\"";
//             }
//         }
//
//         std::cout << "\n解析为GLFW键值：";
//         if (!combo.keys.empty()) {
//             bool first = true;
//             for (int key: combo.keys) {
//                 if (!first) std::cout << ", ";
//                 std::cout << glfwKeyToString(key);
//                 first = false;
//             }
//         } else {
//             std::cout << "无有效键";
//         }
//         std::cout << "\n-------------------------\n";
//     }
//
//     glfwTerminate();
//     return 0;
// }
