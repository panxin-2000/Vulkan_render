//
// Created by 潘鑫 on 2025/12/11.
//

#include <iostream>
// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <valarray>
#include <GLFW/glfw3.h>


#include "labyrinth.h"

#include "render_object_manage.h"


#include "base_event.h"
#include "event_queue_mange.h"
#include "base_observer.h"
#include "input_device_manage.h"
#include "observer_manage.h"
#include "ECS.h"
#include "entity_name_component.h"
#include "input_component.h"
#include "model_matrix_component.h"
#include "scene_component.h"


// 只要鼠标动了就会调用这里
void mouse_callback(GLFWwindow *window, double x_pos, double y_pos) {
    // x_pos = ((x_pos / get_win_WIDTH()) - 0.5f) * 2, y_pos = ((y_pos / get_win_HEIGHT()) - 0.5f) * -2;
    Keyboard_Manage::instance().handle_drag({(float) x_pos, (float) y_pos});
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);


void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    std::cout << "key: " << key << std::endl;

    if (key == GLFW_KEY_UNKNOWN) return;
    // action: GLFW_PRESS（按下）、GLFW_RELEASE（松开）、GLFW_REPEAT（重复按下）
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
        return;
    }
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        Keyboard_Manage::instance().handleKeyDown(key);
    } else if (action == GLFW_RELEASE) {
        Keyboard_Manage::instance().handleKeyUp(key);
    }
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
    std::cout << "x: " << x_pos << " y: " << y_pos << std::endl;
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

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    Keyboard_Manage::instance().handle_scroll({static_cast<float>(xoffset), static_cast<float>(yoffset)});
}

#include "tbb/tbb.h"
GLFWwindow *window;


void on_key_press(const base_event_with_stamp &event) {
    // if (event.key_code == 27) /* 处理退出逻辑 */;
    auto &storage = get_entt_instance().storage<Scene_Component>();

    auto view = get_entt_instance().view<Name_component, Scene_Component, Input_Component>();

    std::vector<entt::entity> all_node_need_check;

    static entt::entity last_work = get_scene_root();
    mouse_position current_position = event.current_position;

    auto &name = view.get<Name_component>(last_work);
    std::cout << "last work name: " << name.name << std::endl;
    if (Scene_Component::check_entity_intersect_point(last_work, current_position))
        if (const auto input = get_entt_instance().try_get<Input_Component>(last_work)) {
            if (input->on_Event != nullptr && input->on_Event(last_work, event) == true) {
                last_work = last_work;
                return;
            }
        }
    last_work = get_scene_root();

    for (auto entity: view) {
        // 这种方式获取组件在内存中是最高效的
        all_node_need_check.push_back(entity);
        auto &name = view.get<Name_component>(entity);
        auto &scene = view.get<Scene_Component>(entity);
        // std::cout << "name: " << name.name
        //         << scene.bounding_box_.centroid_point << scene.bounding_box_.direction_interval << std::endl;
    }
    // 找到当前区域的一个递归栈
    std::vector<entt::entity> UI_stack = UI_stack_intersect(current_position);
    // std::cout << "UI stack size: " << UI_stack.size() << std::endl;
    // for (auto it = UI_stack.rbegin(); it != UI_stack.rend(); ++it) {
    //     auto &name = view.get<Name_component>(*it);
    //     std::cout << "name: " << name.name << std::endl;
    // }
    for (auto it = UI_stack.rbegin(); it != UI_stack.rend(); ++it) {
        if (const auto input = get_entt_instance().try_get<Input_Component>(*it)) {
            if (input->on_Event != nullptr && input->on_Event(*it, event) == true) {
                last_work = *it;
                break;
            }
        }
    }
}


void add_render_windows() {
    int major, minor, rev;
    glfwGetVersion(&major, &minor, &rev);
    std::cout << "GLFW 运行时版本：" << major << "." << minor << "." << rev << std::endl;

    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    window = glfwCreateWindow(get_win_WIDTH(), get_win_HEIGHT(), "LearnOpenGL", nullptr, nullptr);

    // 注册GLFW回调

    entt::dispatcher dispatcher;
    dispatcher.sink<base_event_with_stamp>().connect<&on_key_press>();


    Keyboard_Manage::instance().init_eventQueueMgr(&dispatcher);
    // Keyboard_Manage::instance().register_key_combination("'a'");
    glfwSetWindowFocusCallback(window, glfwFocusCallback); // 窗口焦点回调

    glfwSetKeyCallback(window, glfwKeyCallback); // 键盘事件回调

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // glfwSetCursorEnterCallback(window, function_name);
    glfwSetMouseButtonCallback(window, mouse_button_callback);


    // Game loop
    // 创建循环是没有问题的，但是我需要更改一点内容，去创建另一个线程
    // 另一个线程，完全负责渲染，另一个线程负责准备内容，
    // 在渲染的线程中，检查哪些内容需要更新，然后更新缓存，之后再进行渲染
    // 如果没有需要更新缓存的内容，就不渲染
    std::thread t(start_render_manage_thread, window);

    t.detach();

    while (!glfwWindowShouldClose(window)) {
        glfwWaitEvents();
        if (GLFW_TRUE == glfwWindowShouldClose(window)) {
            break;
        }

        glfwPollEvents();

        dispatcher.update(); // 统一分发执行

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }


    end_render_manage_thread();

    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
}
