//
// Created by 潘鑫 on 2026/1/27.
//

#ifndef HELLO_MAC_VK_RENDER_TO_IMAGE_H
#define HELLO_MAC_VK_RENDER_TO_IMAGE_H
#include <atomic>
#include <mutex>
#include <thread>

#include "create_pipeline.h"
#include "create_shader.h"
#include "engine.h"
#include "transfer_texture_to_gpu.h"
#include "vertex_and_buffer_index.h"
#include "vulkan_build_command_buffer.h"
#include "vulkan_backend.h"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE


#include "descriptor_pool.h"
#include "name_component.h"
#include "pipeline_layout.h"
#include "vulkan_render_manage.h"
#include "sets_and_bindings_layout.h"


class vk_render_GPU {
    mutable std::mutex mtx;
    std::atomic<bool> have_object_need_update = false;
#define not_start 0
#define running 1
#define need_stop 2
    std::atomic<uint32_t> need_render = not_start; // 这里状态有点少了，需要 未开始，运行中，需停止


public:
    void one_cycle(VK_backend &handle) { {
            std::unique_lock<std::mutex> lock(mtx);
            vk_render_queue::instance().execute_update_lambda();
        } {
            const auto view = Render_entt().view<Render_destroy_tag>();
            Render_entt().destroy(view.begin(), view.end()); // 执行销毁程序
        }
        const VkQueryPool queryPool = VK_NULL_HANDLE;

        handle.get_image_to_render();
        const uint64_t time_line = VK_backend::get_current_submit_timeline();
        // 查出哪些物体是需要绘制的，但是命令是需要看阶段的
        reset_current_command_buffer(handle, queryPool, time_line);
        begin_g_buffer_rendering_attachment(handle, time_line); {
            auto view = Render_entt().view<VKR_object_proxy>(entt::exclude<deferred_pass_tag,
                                                                           skybox_tag,
                                                                           UI_2D_tag>);
            // for (const auto it: view) {
            //     auto render_data = view.get<VKR_object_proxy>(it);
            //     build_command_buffer(handle, render_data, time_line);
            // }
        }

        end_rendering(handle);
        g_buffer_attachment_barrier(handle, time_line);


        begin_rendering_attachment(handle, time_line); // 好消息是自己原本的理解已经基本成型了，坏消息是我没有确定分离的位置。
        // 应该先划分不同的 pass 阶段，
        //  deferred  不应该将深度值写入的
        // {
        //     auto view = Render_entt().view<VKR_object_proxy, deferred_pass_tag>();
        //     for (const auto it: view) {
        //         auto render_data = view.get<VKR_object_proxy>(it);
        //         build_deferred_command_buffer(handle, render_data, time_line);
        //     }
        // }

        {
            auto view = Render_entt().view<VKR_object_proxy>();
            for (const auto it: view) {
                auto render_data = view.get<VKR_object_proxy>(it);
                build_command_buffer(handle, render_data, time_line);
            }
        } {
            auto view = Render_entt().view<VKR_object_proxy, translate_tag>();
            for (const auto it: view) {
                auto render_data = view.get<VKR_object_proxy>(it);
                build_command_buffer(handle, render_data, time_line);
            }
        } {
            auto view = Render_entt().view<VKR_object_proxy, UI_2D_tag>();
            for (const auto it: view) {
                auto render_data = view.get<VKR_object_proxy>(it);
                build_command_buffer(handle, render_data, time_line);
            }
        }

        end_rendering(handle);
        end_command_buffer(handle, queryPool, time_line);

        handle.submit_render_queue(time_line);
        handle.copy_image_to_screen();


        // render_object_function();
        clean_need_objects();
        //
    }

    void exit_and_clean(VK_backend &handle) {
        // 需要管理的资源以及删除的顺序
        // blender 中 descriptor_sets_layout 很有意思，在全局的最后才销毁 （中间申请的似乎从不销毁）
        // 一个原因是它关联了三个 内容，另一个原因是整体来说，它的布局很少改变，不会指数增长


        // 先删除（或重置）VkDescriptorSet，后删除 VkDescriptorSetLayout
        // 必须遵循“由实例到定义”的倒序销毁原则

        // 整体的顺序
        // descriptor_pools_
        // pipelines_
        // pipeline_layouts_
        // descriptor_sets_layout
        // shader_modules_
        // buffer_views_
        // buffers_
        // image_views_
        // images_

        VK_CHECK_RESULT_NOT_EXIT(vkDeviceWaitIdle(VK_backend::get().get_device()));

        Render_entt().clear();

        clean_need_objects();

        destroy_descriptorPool();


        // pipeline 建议提前清理
        clean_all_pipeline(handle);
        clean_all_pipeline_layout(handle);
        clean_all_shader_object(handle);
        // VkDescriptorSet
        clean_all_descriptor_sets_layout(handle);

        clean_all_mesh_object(); // 放在这里似乎并不是太好，

        destroy_texture(&handle);


        have_object_need_update = false;
        need_render             = not_start;
    }

    void render_thread(VK_backend &handle) {
        if (need_render == running) {
            return; // 已经在运行中了，直接返回
        }
        need_render = running; // 设置为运行中

        while (need_render == running) {
            one_cycle(handle);
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            LOG_INFO(g_log(), "current finished timeline {}", handle.get_finished_timeline());
        }
        exit_and_clean(handle);
    }

    // 显式同步：即使使用 detach，也应通过原子变量（如 std::atomic<bool>）或信号量
    // 通知子线程退出，并确保其在主线程销毁全局资源前完成清理
    void render_thread_stop() {
        if (need_render == running) {
            need_render = need_stop;
        }
    }

    void render_thread_stop_and_wait() {
        if (need_render == running) {
            need_render = need_stop;
            while (need_render != not_start) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }

    static vk_render_GPU &instance() {
        static vk_render_GPU *instance = nullptr;
        static std::once_flag flag;
        std::call_once(flag, []() {
            instance = new vk_render_GPU();
        });
        return *instance;
    }

private:
    void clean_need_objects() {
        // discard_descriptor_set_map_clean(); // descriptor_pools_
        //                                     pipelines_
        //                                     pipeline_layouts_
        //                                     descriptor_sets_layout
        //                                     shader_modules_
        //                                     buffer_views_
        discard_buffer_map_clean();         //         buffers_
        discard_image_and_view_map_clean(); //  image_views_
        //                                     images_


        // 简单的将内存区域标记为没有内容
        // init_need_objects 再根据需要进行移动或者拼接操作
    }

private:
    vk_render_GPU() {
    }

    ~vk_render_GPU() = default;

public:
    vk_render_GPU(const vk_render_GPU &) = delete;

    vk_render_GPU &operator=(const vk_render_GPU &) = delete;

    vk_render_GPU(vk_render_GPU &&) = delete;

    vk_render_GPU &operator=(vk_render_GPU &&) = delete;
};


#endif //HELLO_MAC_VK_RENDER_TO_IMAGE_H
