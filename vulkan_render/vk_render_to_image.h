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
#include "vulkan_device_handle.h"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE


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

    std::vector<draw_need_vk *> need_render_objects;

public:
    void render_thread(VKDevice &handle) {
        if (need_render == running) {
            return; // 已经在运行中了，直接返回
        }
        need_render = running; // 设置为运行中

        while (need_render == running) {
            {
                std::unique_lock<std::mutex> lock(mtx);
                init_need_objects(handle); // 主要是复制内存的操作
                update_need_objects();
            }
            handle.get_one_image_can_render();

            // 查出哪些物体是需要绘制的，但是命令是需要看阶段的
            begin_rendering(handle); // 好消息是自己原本的理解已经基本成型了，坏消息是我没有确定分离的位置。
            // 应该先划分不同的 pass 阶段，
            for (auto render_data: need_render_objects) {
                build_command_buffer(handle, *render_data);
            }
            end_rendering(handle);
            handle.put_one_image_to_screen();

            // render_object_function();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            clean_need_objects();
        }


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

        VK_CHECK_RESULT_NOT_EXIT(vkDeviceWaitIdle(VKDevice::get().get_device()));

        handle.destroy_descriptorPool();


        vkDestroyCommandPool(handle.get_device(), handle.get_command_pool(), nullptr);

        // pipeline 建议提前清理
        clean_all_pipeline(handle);
        clean_all_pipeline_layout(handle);
        clean_all_shader_object(handle);
        // VkDescriptorSet
        clean_all_descriptor_sets_layout(handle);

        clean_all_mesh_object(handle);

        destroy_texture(&handle);


        have_object_need_update = false;
        need_render             = not_start;
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

private
:
    void init_need_objects(VKDevice &handle) {
        while (true) {
            // 能编译过，但是漏洞百出 ，先预防一手，去制作一些日志
            auto option_temp = vk_render_queue::instance().get_need_init();
            if (option_temp.has_value()) {
                auto render_data = option_temp.value();
                LOG_INFO(g_log(), "get {} from vk_render_queue", render_data->debug_name);
                need_render_objects.push_back(render_data);
            } else {
                break;
            }
        }
    }


    void update_need_objects() {
        while (true) {
            auto render_data = vk_render_queue::instance().get_need_update();
            if (render_data.has_value()) {
                LOG_INFO(g_log(), "get {} from vk_render_queue", render_data.value()->debug_name);
            } else {
                break;
            }
        }
        // 内存内容的更新
        // 先查找放置在哪里来
        // 之后再更新数据
    }

    void clean_need_objects() {
        while (true) {
            auto render_data = vk_render_queue::instance().get_need_clean();
            if (render_data.has_value()) {
                LOG_INFO(g_log(), "get {} from vk_render_queue", render_data.value()->debug_name);
            } else {
                break;
            }
        }
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
