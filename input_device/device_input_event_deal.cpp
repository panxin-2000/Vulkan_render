#include <GLFW/glfw3.h>
#include "../event/base_event.h"
#include "entity_name_component.h"
#include "global_singleton.h"
#include "input_component.h"
#include "../event/input_device_manage.h"
#include "scene_component.h"


void glfwFocusCallback(GLFWwindow *window, int focused);

void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

void mouse_callback(GLFWwindow *window, double x_pos, double y_pos);

void scroll_callback(GLFWwindow *window, double x_offset, double y_offset);

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

entt::dispatcher dispatcher;

void on_key_press(const base_event_with_stamp &event);


void deal_glfw_event() {
    dispatcher.update();
}


void register_glfw(GLFWwindow *window) {
    dispatcher.sink<base_event_with_stamp>().connect<&on_key_press>();


    Keyboard_Manage::instance().init_eventQueueMgr(&dispatcher);
    glfwSetWindowFocusCallback(window, glfwFocusCallback); // 窗口焦点回调
    glfwSetKeyCallback(window, glfwKeyCallback);           // 键盘事件回调

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // glfwSetCursorEnterCallback(window, function_name);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
}


void on_key_press(const base_event_with_stamp &event) {
    // if (event.key_code == 27) /* 处理退出逻辑 */;
    // auto &storage = g_entt().storage<Scene_Component>();

    const auto view = g_entt().view<Name_component, Scene_Component, Input_Component>();

    std::vector<entt::entity> all_node_need_check;

    static entt::entity last_work         = get_scene_root();
    const mouse_position current_position = event.current_position;

    // 鼠标按下时进入模态，移动时，持续模态，鼠标松开时 完成模态 ，按下 ESC 键时，取消模态（ 取消后按键依旧按下，处理需谨慎）
    // 按下 ESC 键时，取消操作，模态已经在，之后的时间不处理，只等鼠标松开取消模态
    auto &name = view.get<Name_component>(last_work);
    // std::cout << "last work name: " << name.name << std::endl;
    if (Scene_Component::check_entity_intersect_point(last_work, current_position))
        if (const auto input = g_entt().try_get<Input_Component>(last_work)) {
            if (input->on_Event != nullptr) {
                auto status = input->on_Event(last_work, event);
                if (OPERATOR_RUNNING_MODAL & status) {
                    last_work = last_work;
                    return;
                } else if (OPERATOR_FINISHED & status) {
                    last_work = get_scene_root();
                    return;
                }
            }
        }

    for (auto entity: view) {
        // 这种方式获取组件在内存中是最高效的
        all_node_need_check.push_back(entity);
        // auto &name = view.get<Name_component>(entity);
        // auto &scene = view.get<Scene_Component>(entity);
        // std::cout << (uint32_t) entity << " name: " << name.name
        //         << scene.bounding_box_.centroid_point << scene.bounding_box_.direction_interval
        //         << (uint32_t) scene.get_parent() << std::endl;
    }
    // 找到当前区域的一个递归栈
    std::vector<entt::entity> UI_stack = UI_stack_intersect(current_position);
    // std::cout << "UI stack size: " << UI_stack.size() << std::endl;
    // for (auto it = UI_stack.rbegin(); it != UI_stack.rend(); ++it) {
    //     auto &name = view.get<Name_component>(*it);
    //     std::cout << "name: " << name.name << std::endl;
    // }
    for (auto it = UI_stack.rbegin(); it != UI_stack.rend(); ++it) {
        if (const auto input = g_entt().try_get<Input_Component>(*it)) {
            if (input->on_Event != nullptr) {
                auto status = input->on_Event(*it, event);
                if (OPERATOR_RUNNING_MODAL & status) {
                    last_work = *it;
                    break;
                } else if (OPERATOR_PASS_THROUGH & status) {
                    continue;
                } else {
                    break;
                }
            }
        }
    }
}


// GLFW窗口焦点回调（失去焦点时清空按键状态）
void glfwFocusCallback(GLFWwindow *window, int focused) {
    if (focused == GLFW_FALSE) {
        Keyboard_Manage::instance().clear_focus();
        glfwSetKeyCallback(window, nullptr); // 键盘事件回调

        // glfwSetInputMode(window, GLFW_CURSOR, nullptr);
        glfwSetCursorPosCallback(window, nullptr);
        glfwSetScrollCallback(window, nullptr);
        // glfwSetCursorEnterCallback(window, nullptr);
        glfwSetMouseButtonCallback(window, nullptr);
    } else {
        Keyboard_Manage::instance().set_focus();
        glfwSetKeyCallback(window, glfwKeyCallback); // 键盘事件回调

        // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);
        // glfwSetCursorEnterCallback(window, function_name);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
    }
}

void scroll_callback(GLFWwindow *window, double x_offset, double y_offset) {
    Keyboard_Manage::instance().handle_scroll({static_cast<float>(x_offset), static_cast<float>(y_offset)});
}


// Window dimensions


/**
 * 鼠标点击之后才会调用这个函数
 * @param window
 * @param button
 * @param action
 * @param mods
 */
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    double x_pos;
    double y_pos;

    glfwGetCursorPos(window, &x_pos, &y_pos);
    LOG_INFO(g_log(), "mouse x: {:4d} y: {:4d} ", (int)x_pos, (int)y_pos);
    // 由窗口具体的窗口自己去更改吧。
    // x_pos = ((x_pos / get_win_WIDTH()) - 0.5f) * 2, y_pos = ((y_pos / get_win_HEIGHT()) - 0.5f) * -2;
    // 更改坐标系的范围，x轴是从左到右，范围是-1到1之间，y轴是从下到上，范围是-1到1之间
#define key_instance Keyboard_Manage::instance()
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        key_instance.handle_mouse_click_left({static_cast<float>(x_pos), static_cast<float>(y_pos)});
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        key_instance.handle_mouse_release_left({static_cast<float>(x_pos), static_cast<float>(y_pos)});
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        key_instance.handle_mouse_click_right({static_cast<float>(x_pos), static_cast<float>(y_pos)});
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        key_instance.handle_mouse_release_right({static_cast<float>(x_pos), static_cast<float>(y_pos)});
    }
#undef key_instance
}


void glfwKeyCallback(GLFWwindow *window, const int key, int scancode, const int action, int mods) {
    // std::cout << "key: " << key << std::endl;

    if (key == GLFW_KEY_UNKNOWN) return;
    // action: GLFW_PRESS（按下）、GLFW_RELEASE（松开）、GLFW_REPEAT（重复按下）
    // if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    //     glfwSetWindowShouldClose(window, GL_TRUE);
    //     return;
    // }
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        Keyboard_Manage::instance().handleKeyDown(key);
    } else if (action == GLFW_RELEASE) {
        Keyboard_Manage::instance().handleKeyUp(key);
    }
}


// 只要鼠标动了就会调用这里
void mouse_callback(GLFWwindow *window, const double x_pos, const double y_pos) {
    // x_pos = ((x_pos / get_win_WIDTH()) - 0.5f) * 2, y_pos = ((y_pos / get_win_HEIGHT()) - 0.5f) * -2;
    Keyboard_Manage::instance().handle_drag({static_cast<float>(x_pos), static_cast<float>(y_pos)});
}
