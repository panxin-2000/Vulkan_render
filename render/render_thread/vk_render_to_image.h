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


struct float4 {
    float x, y, z, w;
};

struct FrustumCorners {
    float4 corners[8];
};

struct FrustumPlanes {
    /* [0] left
     * [1] right
     * [2] bottom
     * [3] top
     * [4] near
     * [5] far */
    float4 planes[6];
};

struct ViewCullingData {
    /** \note float3 array padded to float4. */
    /** Frustum corners. */
    FrustumCorners frustum_corners;
    FrustumPlanes frustum_planes;
    float4 bound_sphere;
};


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

        // 阴影的 pass
        {
            // g_buffer_image_indices 这是需要看看怎么传递进入其中
            const auto view = Render_entt().view<shadow_pass_tag>();
            if (!view.empty()) {
                begin_shadow_pass(handle, time_line);

                // 中间需要添加 被光 照 到的物体，能产生阴影的物体
                // 这里的时候发生了一点改变，为什么呢？ 单个 mesh 需要多个不同的 render pass
                // 这也意味着 需要 它 开始需要多个 shader 了
                // 然后的问题就是，多个 shader 中，哪些是共同的部分，哪些不是？
                // 这里开始需要什么了呢？ 光，灯光是需要的部分 ，物体在世界空间的位姿也是需要的部分，
                // 不同的是输出的内容，以及输出的内容在什么时候再次被需要
                // model_matrix 是 在多个 shader 都保持不变的值。

                // 拿到z值，并反推出在世界空间中的位置需要 view 和 projection 的逆矩阵
                // 每个灯光，都需要其 view 和 projection 矩阵
                // 这个时候，其实应该是可以考虑关于遮挡部分的内容了，比如，
                // 计算平头截体，然后将不在其中的物体给排除出去。
                // 这个时候需要什么呢？ 物体的包围盒，model ,之后 再与 平头截体进行相交的判断
                // 之后再是什么呢？ 看看如何将这部分的计算放到GPU中计算

                end_rendering(handle);
                shadow_pass_barrier(handle, time_line);
            }
        }
        auto g_buffer_image_indices = begin_g_buffer_rendering_attachment(handle, time_line); {
            auto view = Render_entt().view<deferred_pass_tag>();
            if (!view.empty()) {
                auto view_opacity = Render_entt().view<opacity_tag>();
                for (const auto it: view_opacity) {
                    build_command_buffer(handle, it, time_line);
                }
            }
        }

        end_rendering(handle);
        g_buffer_attachment_barrier(handle, time_line);


        begin_rendering_attachment(handle, time_line); // 好消息是自己原本的理解已经基本成型了，坏消息是我没有确定分离的位置。
        // 应该先划分不同的 pass 阶段，
        //  deferred  不应该将深度值写入的
        {
            // g_buffer_image_indices 这是需要看看怎么传递进入其中
            auto view = Render_entt().view<deferred_pass_tag>();
            for (const auto it: view) {
                build_command_buffer(handle, it, time_line);
            }
        } {
            auto view = Render_entt().view<Mesh, opacity_tag>();
            for (const auto it: view) {
                build_command_buffer(handle, it, time_line);
            }
        } {
            auto view = Render_entt().view<Mesh, skybox_tag>();
            for (const auto it: view) {
                build_command_buffer(handle, it, time_line);
            }
        } {
            auto view = Render_entt().view<Mesh, translate_tag>();
            for (const auto it: view) {
                build_command_buffer(handle, it, time_line);
            }
        } {
            auto view = Render_entt().view<Mesh, UI_2D_tag>();
            for (const auto it: view) {
                build_command_buffer(handle, it, time_line);
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
            // LOG_INFO(g_log(), "current finished timeline {}", handle.get_finished_timeline());
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
