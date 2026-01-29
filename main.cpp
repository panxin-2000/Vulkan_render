/* Copyright (c) 2025-2026, Sascha Willems
 * SPDX-License-Identifier: MIT
 */

#include <GLFW/glfw3.h>

#include <thread>

#include "vulkan_render/backend.h"
#include "vulkan_render/vulkan_render_manage.h"


#include "vulkan_device_handle.h"
VKDevice handle;


int main(int argc, char *argv[]) {
    handle.init_device_handle();
    // Window and surface
    render_thread_start(handle);
    // Render loop
    while (!glfwWindowShouldClose(handle.window_)) {
        glfwPollEvents();
        // Event polling
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    render_thread_stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    handle.destroy();
}
