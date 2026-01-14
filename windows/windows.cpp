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


const GLuint WIDTH = 800, HEIGHT = 600;


// 只要鼠标动了就会调用这里
void mouse_callback(GLFWwindow *window, double x_pos, double y_pos) {
    x_pos = ((x_pos / WIDTH) - 0.5f) * 2, y_pos = ((y_pos / HEIGHT) - 0.5f) * -2;
    Keyboard_Manage::instance().handle_drag({(float) x_pos, (float) y_pos});
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);


void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // action: GLFW_PRESS（按下）、GLFW_RELEASE（松开）、GLFW_REPEAT（重复按下）
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        Keyboard_Manage::instance().handleKeyDown(key);
    } else if (action == GLFW_RELEASE) {
        Keyboard_Manage::instance().handleKeyUp(key);
    }
}

// GLFW窗口焦点回调（失去焦点时清空按键状态）
void glfwFocusCallback(GLFWwindow *window, int focused) {
    if (!focused) {
        Keyboard_Manage::instance().clearState(); // 防止窗口失焦后按键状态残留
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
    x_pos = ((x_pos / WIDTH) - 0.5f) * 2, y_pos = ((y_pos / HEIGHT) - 0.5f) * -2;
    // 更改坐标系的范围，x轴是从左到右，范围是-1到1之间，y轴是从下到上，范围是-1到1之间
    std::cout << "x: " << x_pos << " y: " << y_pos << std::endl;
#define key_instance Keyboard_Manage::instance()
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        key_instance.handle_mouse_click_left({(float) x_pos, (float) y_pos});
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        key_instance.handle_mouse_release_left({(float) x_pos, (float) y_pos});
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        key_instance.handle_mouse_click_right({(float) x_pos, (float) y_pos});
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        key_instance.handle_mouse_release_right({(float) x_pos, (float) y_pos});
    }
#undef key_instance
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    Keyboard_Manage::instance().handle_scroll({(float) xoffset, (float) yoffset});
}

#include "tbb/tbb.h"
GLFWwindow *window;


void on_key_press(const base_event_with_stamp &event) {
    // if (event.key_code == 27) /* 处理退出逻辑 */;
    auto &storage = get_entt_instance().storage<Position_component>();

    auto view = get_entt_instance().view<Position_component>();
    std::cout << "View size: " << view.size() << std::endl;

    for (auto entity: view) {
        // 这种方式获取组件在内存中是最高效的
        auto &pos = view.get<Position_component>(entity);
        const EventType temp_type = event.type;
        switch (temp_type) {
            case EventType::key_combination:
                break;
            case EventType::mouse_click_left:
                // 保存首次点击的位置
                break;
            case EventType::mouse_click_right:
                // 保存首次点击的位置
                break;
            case EventType::scroll:
                pos.set_zoom(event);
                pos.update_position();

                break;
            case EventType::drag:
                pos.set_position_offset(event);
                pos.update_position();
                break;

            default: ;
        }
    }
}


void add_render_windows() {
    auto view = get_entt_instance().view<Position_component>();
    std::cout << "View size: " << view.size() << std::endl;

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
    window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);

    // 注册GLFW回调

    entt::dispatcher dispatcher;
    dispatcher.sink<base_event_with_stamp>().connect<&on_key_press>();


    Keyboard_Manage::instance().init_eventQueueMgr(&dispatcher);
    // Keyboard_Manage::instance().register_key_combination("'a'");
    glfwSetKeyCallback(window, glfwKeyCallback);           // 键盘事件回调
    glfwSetWindowFocusCallback(window, glfwFocusCallback); // 窗口焦点回调


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
        // glfwPollEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        dispatcher.update(); // 统一分发执行

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }


    // observe_manage_instance::instance().observer_manage_thread_close();
    end_render_manage_thread();

    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
}
