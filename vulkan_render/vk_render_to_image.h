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
#include "descriptor.h"
#include "engine.h"
#include "transfer_texture_to_gpu.h"
#include "vertex_and_buffer_index.h"
#include "vulkan_build_command_buffer.h"
#include "vulkan_device_handle.h"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "logic_render_data.h"
#include "pipeline_layout.h"
#include "vulkan_render_manage.h"
#include "sets_and_bindings_layout.h"

glm::vec3 camPos{0.0f, 0.0f, -6.0f};
glm::vec3 objectRotations[3]{};

std::vector<VkDescriptorSet> g_hjk;
const uint32_t WIDTH  = 1280; // 也是需要更改的
const uint32_t HEIGHT = 720;

void update_shader_data(Engine &engine) {
    // 我想更改某些内容的话，需要从这里下手
    std::vector<ShaderData> ShaderDatas;
    ShaderData shaderData;


    shaderData.projection = glm::perspective(glm::radians(45.0f), (float) WIDTH / (float) HEIGHT, 0.1f, 32.0f);
    shaderData.view       = glm::translate(glm::mat4(1.0f), camPos);
    for (auto i = 0; i < 3; i++) {
        auto instancePos    = glm::vec3((float) (i - 1) * 3.0f, 0.0f, 0.0f);
        shaderData.model[i] = glm::translate(glm::mat4(1.0f), instancePos) * glm::mat4_cast(
                                   glm::quat(objectRotations[i]));
    }
    ShaderDatas.push_back(shaderData);

    shaderData.projection = glm::mat4(1.0f);
    shaderData.view       = glm::mat4(1.0f);
    for (auto i = 0; i < 3; i++) {
        auto instancePos    = glm::vec3((float) (i - 1) * 3.0f, 0.0f, 0.0f);
        shaderData.model[i] = glm::mat4(1.0f);
    }
    ShaderDatas.push_back(shaderData);
    memcpy(engine.get_current_shader_data_buffer().mapped, ShaderDatas.data(),
           ShaderDatas.size() * sizeof(ShaderData));
}

#include <map>

class vk_render_GPU {
    mutable std::mutex mtx;
    std::atomic<bool> have_object_need_update = false;
#define not_start 0
#define running 1
#define need_stop 2
    std::atomic<uint32_t> need_render = not_start; // 这里状态有点少了，需要 未开始，运行中，需停止

    std::vector<logic_render_data *> need_render_objects;

public:
    void render_thread(VKDevice &handle) {
        if (need_render == running) {
            return; // 已经在运行中了，直接返回
        }
        need_render = running; // 设置为运行中

        Engine engine;
        engine.init();
        // 目的是为了简化函数，
        // Texture images
        create_textures_to_gpu(handle, handle.get_command_pool());


        while (need_render == running) {
            {
                std::unique_lock<std::mutex> lock(mtx);
                init_need_objects(handle); // 主要是复制内存的操作
                update_need_objects();
            }
            engine.get_one_image_can_render();
            update_shader_data(engine); // 这里是一个需要同步的点

            // 查出哪些物体是需要绘制的，但是命令是需要看阶段的
            begin_rendering(engine);
            // 应该先划分不同的 pass 阶段，
            for (auto render_data: need_render_objects) {
                auto shaderStages = find_graphics_shader_module(handle, render_data->vertexPath_,
                                                                render_data->fragmentPath_,
                                                                render_data->geometryPath_);
                if (shaderStages.empty() == true) {
                    continue;
                }
                auto shader_key = get_shader_key(render_data->vertexPath_,
                                                 render_data->fragmentPath_,
                                                 render_data->geometryPath_);

                auto pipelineLayout         = find_pipeline_layout(handle, shader_key);
                const auto vertexInputState = vertex_input_position_normal_uv();
                auto pipeline_t             = find_pipeline(handle, render_data, pipelineLayout, shaderStages,
                                                VKDevice::get().get_pipeline_map());
                if (pipeline_t == VK_NULL_HANDLE) {
                    continue;
                }
                auto mesh = find_mesh(render_data, VKDevice::get().get_mesh_map());
                if (mesh == nullptr) {
                    continue;
                }
                int i = 0;
                if (render_data->debug_name == "blender Suzanne") {
                    i = 0;
                    build_command_buffer(engine, pipeline_t, pipelineLayout, g_hjk[0], *mesh,
                                         i * sizeof(ShaderData));
                } else {
                    i = 1;
                }
            }
            end_rendering(engine);
            engine.put_one_image_to_screen();

            // render_object_function();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            clean_need_objects();
        }


        // descriptor.Destroy(); //

        // 需要管理的资源以及删除的顺序
        // buffer_views_            // 这四个建议放置到 descriptor set 之后
        // buffers_                 // 这四个建议放置到 descriptor set 之后
        // image_views_             // 这四个建议放置到 descriptor set 之后
        // images_                  // 这四个建议放置到 descriptor set 之后
        // shader_modules_
        // pipelines_
        // pipeline_layouts_
        // descriptor_sets_layout    // 这个也需要去清理， blender 中很有意思，在全局的最后才销毁
        // 一个原因是它关联了三个 内容，另一个原因是整体来说，它的布局很少改变，不会指数增长
        // descriptor_pools_   // 最后这个，有点 不同 VkDescriptorSetLayout
        // 先删除（或重置）VkDescriptorSet，后删除 VkDescriptorSetLayout
        // 必须遵循“由实例到定义”的倒序销毁原则

        // pipeline 建议提前清理
        clean_all_pipeline(handle);
        clean_all_pipeline_layout(handle);
        clean_all_shader_object(handle);
        // VkDescriptorSet
        clean_all_descriptor_sets_layout(handle);

        clean_all_mesh_object(handle);

        destroy_texture(&handle);

        engine.destroy();


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
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
                find_graphics_shader_module(handle, render_data->vertexPath_,
                                            render_data->fragmentPath_,
                                            render_data->geometryPath_);
                auto organized_sets_and_bindings =
                        organize_graphics_descriptor_set_and_binding_layouts(render_data->vertexPath_,
                                                                             render_data->fragmentPath_,
                                                                             render_data->geometryPath_);
                auto shader_key = get_shader_key(render_data->vertexPath_,
                                                 render_data->fragmentPath_,
                                                 render_data->geometryPath_);

                const auto descriptor_sets_layout =
                        create_descriptor_sets_layout(handle, shader_key, organized_sets_and_bindings);
                auto sets_flags      = create_descriptor_sets_flags(handle, organized_sets_and_bindings);
                auto pipeline_layout = create_pipeline_layout(handle, shader_key, descriptor_sets_layout);


                if (render_data->debug_name == "blender Suzanne") {
                    auto descriptor_set_texture = allocate_descriptor_sets(handle, descriptor_sets_layout[0],
                                                                           sets_flags[0]);
                    g_hjk = descriptor_set_texture;
                    update_descriptor_sets(handle, handle.get_bindless_textures(), descriptor_set_texture);
                    // 更新应该被拆出来， 放到需要的位置再上传
                }
                create_mesh(handle, render_data, VKDevice::get().get_mesh_map());

                // create_element_buffer(render_data.value()->indices_, &indices_map_);
                // create_texture(render_data.value()->textures, &texture_map_);
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
