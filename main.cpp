/* Copyright (c) 2025-2026, Sascha Willems
 * SPDX-License-Identifier: MIT
 */

#include <GLFW/glfw3.h>

#include <thread>
#include "vulkan_render/backend.h"
#include "vulkan_render/vulkan_render_manage.h"
#include "vulkan_device_handle.h"
#include "event/base_event.h"
#include "labyrinth.h"
#include "UI/UI_block.h"
#include "UI/UI_button.h"

#include "global_singleton.h"

void register_glfw(GLFWwindow *window);

void deal_glfw_event();

VKDevice handle;


int main(int argc, char *argv[]) {
    LOG_INFO(g_log(), "Hello from {}!", "Quill v11.0.2");

    handle.init_device_handle();
    render_thread_start(handle);

    // auto entity = get_entt_instance().create();
    // get_entt_instance().emplace<Labyrinth>(entity, "迷宫", entity);
    register_glfw(handle.window_);

    auto block_entity = UI_block("功能块", 10, 10, 220, 220);
    add_button(block_entity, "按钮1", 420, 420, 480, 480);
    add_button(block_entity, "按钮2", 35, 20, 145, 130);


    // Render loop
    while (!glfwWindowShouldClose(handle.window_)) {
        glfwWaitEvents();
        if (GLFW_TRUE == glfwWindowShouldClose(handle.window_)) {
            break;
        }
        glfwPollEvents();  // Event polling
        deal_glfw_event(); // 统一分发执行
        {
            auto view = g_entt().view<Destroy_tag>();   //得到哪些需要销毁
            g_entt().destroy(view.begin(), view.end()); // 执行销毁程序
        } {
            auto view = g_entt().view<Position_update_tag>(); //得到哪些需要销毁
            // g_entt().destroy(view.begin(), view.end());       // todo : 添加新的函数
        }


        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    render_thread_stop_and_wait();

    handle.destroy();
}
