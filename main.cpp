/* Copyright (c) 2025-2026, Sascha Willems
 * SPDX-License-Identifier: MIT
 */

#include <GLFW/glfw3.h>

#include <thread>
#include "render_thread/backend.h"
#include "render_thread/vulkan_render_manage.h"
#include "vulkan_device_handle.h"
#include "event/base_event.h"
#include "labyrinth.h"
#include "UI/UI_block.h"
#include "UI/UI_button.h"

#include "global_singleton.h"


#include <DirectXMath.h>
#include <iostream>

#include "descriptor_pool.h"
#include "update_push_constants_data.h"
#include "UI/3d_model_display.h"

void register_glfw(GLFWwindow *window);

void deal_glfw_event();


inline void sync_render_data_to_render_thread() {
    // 应该不止更新 position，还有很多的都需要更新

    {
        // 就是检查一下，已经给过 渲染线程，就添加一个 lambda 更新部分内容就好
        // global 相关的内容尽量只能偏移，
        const auto view = g_entt().view<global_uniform_buffer_update>();
        for (const auto &it: view) {
            update_global_bindings_to_descriptor_sets(it);
            g_entt().remove<global_uniform_buffer_update>(it);
        }
    } {
        const auto view = g_entt().view<uniform_buffer_update>();
        for (const auto &it: view) {
            update_object_bindings_to_descriptor_sets(it);
            g_entt().remove<uniform_buffer_update>(it);
        }
    } {
        const auto view = g_entt().view<std::shared_ptr<VKR_object_proxy> >();
        // 位置发生了更新，需要讲更新传递出去
        for (const auto it: view) {
            auto temp_des = get_descriptor_sets(it);

            auto lambda = [temp_des](const std::shared_ptr<VKR_object_proxy> &proxy) {
                proxy->vk_descriptor_set = temp_des;
            };
            update_VKR_object_proxy(it, lambda);
        }
    } {
        const auto view = g_entt().view<add_to_render_tag>(entt::exclude<std::shared_ptr<VKR_object_proxy> >);
        for (const auto &it: view) {
            create_VKR_object_proxy(it); // 因为这里没有区分。全部都在场景的根节点之下
        }
    } {
        const auto view = g_entt().view<Rect_2D_transform>();
        for (const auto it: view) {
            auto pos    = view.get<Rect_2D_transform>(it);
            auto offset = pos.get_bounding_box();
            // LOG_INFO(g_log(), "name {}  offset x {} y {}", get_entity_name(it), offset.min_point.x, offset.min_point.y);
        }
    }
    update_object_offset();
}


int main(int argc, char *argv[]) {
    DirectX::XMVECTOR v = DirectX::XMVectorSet(1.0f, 2.0f, 3.0f, 4.0f);
    std::cout << "DirectXMath Integrated Successfully on Mac!" << std::endl;
    // std::cout << " UI_component.h:111  " << std::endl; // 是文件的路径就可以在clion中直接点击显示
    LOG_INFO(g_log(), "Hello from {}!", "Quill v11.0.2");
    auto &handle = VK_handle::get();
    handle.engine_init(); // 必须单独调用，不能在 std::call_once 中 ，否则会死锁
    init_current_descriptor_pool();

    render_thread_start(handle);

    // auto entity = get_entt_instance().create();
    // get_entt_instance().emplace<Labyrinth>(entity, "迷宫", entity);
    register_glfw(handle.get_window());

    UI_block("按钮1", 0, 0, 60, 60);
    UI_block("功能块", 0, 0, 50, 200);
    UI_block("按钮2", 0, 0, 145, 130);


    object_3d_model("blender Suzanne", "assets/suzanne.obj");

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
    g_entt().clear();

    render_thread_stop_and_wait();

    // 全局的 push_constants 的 buffer ,最后在这里销毁稍微有点不太好。
    auto &buffer = get_uniform_buffer();
    buffer->destroy_buffer();

    discard_buffer_map_clean();
    handle.engine_destroy();
    handle.destroy();
}
