//
// Created by 潘鑫 on 2025/12/11.
//

#include "windows.h"


// Function prototypes
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

void add_render_windows() {
    glfwInit();
    GLFWwindow *window;

    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);
    std::thread t(start_render_manage_thread, window);
    t.detach();


    while (!glfwWindowShouldClose(window)) {
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    end_render_manage_thread();

    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
}
