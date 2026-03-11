/* Copyright (c) 2025-2026, Sascha Willems
 * SPDX-License-Identifier: MIT
 */

#include <GLFW/glfw3.h>

#include <thread>
#include "render_thread/backend.h"
#include "vulkan_device_handle.h"
#include "event/base_event.h"
#include "labyrinth.h"
#include "UI/UI_block.h"
#include "UI/UI_button.h"

#include "global_singleton.h"
#include <iostream>
#include "descriptor_pool.h"
#include "sync_proxy_to_render_thread.h"
#include "update_push_constants_data.h"
#include "UI/3d_model_display.h"

void register_glfw(GLFWwindow *window);

void deal_glfw_event();


inline entt::entity add_render_pass(const std::string &name) {
    entt::entity entity_ = g_entt().create();
    g_entt().emplace<Name_component>(entity_, name + "deferred_pass");

    g_entt().emplace<VKR_shader_paths>(entity_,
                                       "/Users/panxin/CLionProjects/hello_mac/render/shader/deferred.vert.spv",
                                       "/Users/panxin/CLionProjects/hello_mac/render/shader/deferred.frag.spv",
                                       "", "");

    // 更新物体的模型矩阵

    world_root_add_child(entity_);

    g_entt().emplace_or_replace<add_to_render_tag>(entity_);
    return entity_;
}


int main(int argc, char *argv[]) {
    // std::cout << " UI_component.h:111  " << std::endl; // 是文件的路径就可以在clion中直接点击显示
    LOG_INFO(g_log(), "Hello from {}!", "Quill v11.0.2");
    auto &handle = VK_handle::get();
    handle.engine_init(); // 必须单独调用，不能在 std::call_once 中 ，否则会死锁
    init_current_descriptor_pool();


    render_thread_start(handle);

    register_glfw(handle.get_window());
    // object_3d_model("Damaged Helmet", "assets/DamagedHelmet.gltf",
    //                 {0.0f, 0.0f, 0.0f},
    //                 {0.7071068286895752, 0.7071068286895752, 0, 0}); // 选择数据暂时是写死的

    // UI_block("按钮1", 0, 0, 60, 60);
    // UI_block("功能块", 0, 0, 50, 200);
    // UI_block("按钮2", 0, 0, 145, 130);
    // 3d 模型
    // {
    //     auto entity = object_3d_model("blender Suzanne", "assets/suzanne.obj", {-3.0f, 0.0f, 0.0f});
    //     set_render_picture(entity, "samplerColor", "assets/suzanne0.ktx");
    // } {
    //     auto entity = object_3d_model("blender Suzanne", "assets/suzanne.obj", {3.0f, 0.0f, 0.0f});
    //     set_render_picture(entity, "samplerColor", "assets/suzanne1.ktx");
    // }

    add_render_pass("blank");


    // Render loop
    while (!glfwWindowShouldClose(handle.get_window())) {
        glfwWaitEvents();
        if (GLFW_TRUE == glfwWindowShouldClose(handle.get_window())) {
            break;
        }
        glfwPollEvents();  // Event polling
        deal_glfw_event(); // 统一分发执行

        clean_render_entity(); {
            auto view = g_entt().view<Destroy_tag>();   //得到哪些需要销毁，销毁之后不再显示 // 实体销毁和销毁显示还是需要区分的
            g_entt().destroy(view.begin(), view.end()); // 执行销毁程序
        }

        sync_render_data_to_render_thread();

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    g_entt().clear(); // 必须先清理， root entity 会占有一部分资源，需要先清理

    render_thread_stop_and_wait();

    // 全局的 push_constants 的 buffer ,最后在这里销毁稍微有点不太好。
    auto &buffer = get_uniform_buffer();
    buffer->destroy_buffer();

    handle.engine_destroy();
    handle.destroy();
}


void test_projection_matrix() {
    auto entity = object_3d_model("triangle", "", {0.0f, 0.0f, 0.0f});
    add_geometry_data(entity, {-0.5f, -0.5f, 0.0f}, {0.5f, -0.5f, 0.0f}, {0.0f, 0.5f, 0.0f});
}
