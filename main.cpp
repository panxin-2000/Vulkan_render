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


#include <DirectXMath.h>
#include <iostream>

void register_glfw(GLFWwindow *window);

void deal_glfw_event();


int main(int argc, char *argv[]) {
    DirectX::XMVECTOR v = DirectX::XMVectorSet(1.0f, 2.0f, 3.0f, 4.0f);
    std::cout << "DirectXMath Integrated Successfully on Mac!" << std::endl;
    LOG_INFO(g_log(), "Hello from {}!", "Quill v11.0.2");
    auto &handle = VK_handle::get();
    handle.engine_init();


    render_thread_start(handle);

    // auto entity = get_entt_instance().create();
    // get_entt_instance().emplace<Labyrinth>(entity, "迷宫", entity);
    register_glfw(handle.window_);

    auto block_entity = UI_block("功能块", 0, 0, 200, 200);
    add_button(block_entity, "按钮1", 420, 420, 480, 480);
    add_button(block_entity, "按钮2", 35, 20, 145, 130);

    auto render        = new logic_render_data;
    render->debug_name = "blender Suzanne";
    render->mesh_path_ = "assets/suzanne.obj";
    render->set_vertex_shader("/Users/panxin/CLionProjects/hello_mac/render/shader/temp.vert.spv");
    render->set_fragment_shader("/Users/panxin/CLionProjects/hello_mac/render/shader/temp.frag.spv");

    // add_object_to_render(render); // 因为这里没有区分。全部都在场景的根节点之下

    // Render loop
    while (!glfwWindowShouldClose(handle.window_)) {
        glfwWaitEvents();
        if (GLFW_TRUE == glfwWindowShouldClose(handle.window_)) {
            break;
        }
        glfwPollEvents();  // Event polling
        deal_glfw_event(); // 统一分发执行
        clean_render_entity(); {
            auto view = g_entt().view<Destroy_tag>();   //得到哪些需要销毁，销毁之后不再显示 // 实体销毁和销毁显示还是需要区分的
            g_entt().destroy(view.begin(), view.end()); // 执行销毁程序
        }
        // update_UI_position();


        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    g_entt().clear();

    render_thread_stop_and_wait();

    // 全局的 push_constants 的 buffer ,最后在这里销毁稍微有点不太好。
    auto &buffer = get_uniform_buffer();
    buffer->DestroyBuffer();

    discard_buffer_map_clean();
    handle.engine_destroy();
    handle.destroy();
}
