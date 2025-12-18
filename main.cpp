#include <iostream>
// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <valarray>
#include <GLFW/glfw3.h>


#include "labyrinth.h"

#include "render/render_object_manage.h"


#include "event/base_event.h"
#include "event/event_queue_mange.h"
#include "event/base_observer.h"
#include "event/input_device_manage.h"
#include "event/observer_manage.h"

float deltaTime = 0.0f; // 当前帧与上一帧的时间差
float lastFrame = 0.0f; // 上一帧的时间
float zoom = 1.0f; // 上一帧的时间


void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);


void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // action: GLFW_PRESS（按下）、GLFW_RELEASE（松开）、GLFW_REPEAT（重复按下）
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
const GLuint WIDTH = 800, HEIGHT = 600;


float lastX = WIDTH / 2, lastY = HEIGHT / 2;
bool firstMouse = true;


/**
 * 鼠标点击之后才会调用这个函数
 * @param window
 * @param button
 * @param action
 * @param mods
 */
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    float xpos;
    float ypos;
    double double_xpos;
    double double_ypos;
    glfwGetCursorPos(window, &double_xpos, &double_ypos);
    xpos = double_xpos, ypos = double_ypos;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        std::cout << "GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS " << std::endl;
        Keyboard_Manage::instance().handle_mouse_button_left_click({xpos, ypos});
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        std::cout << "GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE " << std::endl;
        Keyboard_Manage::instance().handle_mouse_button_left_release({xpos, ypos});
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        std::cout << "GLFW_MOUSE_BUTTON_RIGHT,GLFW_PRESS " << std::endl;
        Keyboard_Manage::instance().handle_mouse_button_right_click({xpos, ypos});
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        std::cout << "GLFW_MOUSE_BUTTON_RIGHT,GLFW_RELEASE " << std::endl;
        Keyboard_Manage::instance().handle_mouse_button_right_release({xpos, ypos});
    }
}

#include "tbb/tbb.h"
GLFWwindow *window;


int main() {
    tbb::task_group group;
    // Parallel invoke may be called with at least two callable
    tbb::parallel_invoke([]() { std::cout << "Hello" << std::endl; },
                         []() { std::cout << "TBB" << std::endl; });

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

    Keyboard_Manage::instance().init_eventQueueMgr(observe_manage_instance::get_event_queue());
    Keyboard_Manage::instance().register_key_combination("'a'");
    glfwSetKeyCallback(window, glfwKeyCallback); // 键盘事件回调
    glfwSetWindowFocusCallback(window, glfwFocusCallback); // 窗口焦点回调


    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // glfwSetCursorEnterCallback(window, function_name);
    glfwSetMouseButtonCallback(window, mouse_button_callback);


    // Game loop
    // 创建循环是没有问题的，但是我需要更改一点内容，去创建另一个线程
    // 另一个线程，完全负责渲染，另一个线程负责准备内容，
    // 在渲染的线程中，检查哪些内容需要更新，然后更新缓存，之后再进行渲染
    // 如果没有需要更新缓存的内容，就不渲染
    std::thread t(start_render_manage_thread, window);
    std::thread t2(observe_manage_instance::observer_manage_thread);

    t.detach();
    t2.detach();
    auto labyrinth = new Labyrinth(); {
        base_observer<base_event> observer{EventType::key_combination, "'a'"};
        observer.set_deal_function(std::bind(&Labyrinth::run_step, labyrinth, std::placeholders::_1));
        observe_manage_instance::instance().addObserver(observer);
    } {
        base_observer<base_event> observer{EventType::key_combination, "'q'"};
        observer.set_deal_function(std::bind(&Labyrinth::run_init, labyrinth, std::placeholders::_1));
        observe_manage_instance::instance().addObserver(observer);
    } {
        base_observer<base_event> observer{EventType::MouseClick, "mouse_button_left_click"};
        observer.set_deal_function(std::bind(&Labyrinth::deal_event, labyrinth, std::placeholders::_1));
        observe_manage_instance::instance().addObserver(observer);
    } {
        base_observer<base_event> observer{EventType::scroll, "mouse_scroll_zoom"};
        observer.set_deal_function(std::bind(&Labyrinth::set_zoom, labyrinth, std::placeholders::_1));
        observe_manage_instance::instance().addObserver(observer);
    }
    // 下一步做什么呢？ 将 zoom 与鼠标的偏移算出来


    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }


    observe_manage_instance::instance().observer_manage_thread_close();
    end_render_manage_thread();

    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}


void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    Keyboard_Manage::instance().handle_scroll({(float) xoffset, (float) yoffset});
}
