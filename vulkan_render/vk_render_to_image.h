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
#include "descriptor_pool.h"
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
#include "vulkan_render_manage.h"


glm::vec3 camPos{0.0f, 0.0f, -6.0f};
glm::vec3 objectRotations[3]{};


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

    VKDevice *handle_;

public:
    void render_thread(VKDevice &handle) {
        if (need_render == running) {
            return; // 已经在运行中了，直接返回
        }
        handle_     = &handle;
        need_render = running; // 设置为运行中
        Descriptor_Pool descriptor_pool(&handle, 250);
        descriptor_pool.init_Descriptor_Pool();
        Descriptor descriptor(&handle, &descriptor_pool);

        Engine engine(handle);
        engine.init();
        // 目的是为了简化函数，
        // Texture images
        auto textureDescriptors = create_textures_to_gpu(&handle, handle.get_command_pool());

        descriptor.CreateDescriptorSetLayout(textureDescriptors.size());
        descriptor.AllocateDescriptorSets(textureDescriptors.size());
        descriptor.update_descriptor_sets(textureDescriptors);
        // 到这里的时候贴图就更新完毕了
        auto pipelineLayout = descriptor.CreatePipelineLayout();
        // 有点难整理清楚


        while (need_render == running) {
            {
                std::unique_lock<std::mutex> lock(mtx);
                init_need_objects(); // 主要是复制内存的操作
                update_need_objects();
            }
            engine.get_one_image_can_render();
            update_shader_data(engine); // 这里是一个需要同步的点


            begin_rendering(engine);
            for (auto need_render_object: need_render_objects) {
                auto shaderStages = find_graphics_shader_module(*handle_, need_render_object->vertexPath_,
                                                                need_render_object->fragmentPath_,
                                                                need_render_object->geometryPath_);
                if (shaderStages.empty() == true) {
                    continue;
                }
                const auto vertexInputState = vertex_input_position_normal_uv();
                auto pipeline_t             = find_pipeline(*handle_, need_render_object, pipelineLayout, shaderStages,
                                                VKDevice::get().get_pipeline_map());
                auto mesh = find_mesh(need_render_object, VKDevice::get().get_mesh_map());
                if (mesh == nullptr) {
                    continue;
                }
                int i = 0;
                if (need_render_object->debug_name == "blender Suzanne") {
                    i = 0;
                } else {
                    i = 1;
                }
                build_command_buffer(engine, pipeline_t, pipelineLayout, descriptor, *mesh,
                                     i * sizeof(ShaderData));
            }
            end_rendering(engine);
            engine.put_one_image_to_screen();

            // render_object_function();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            clean_need_objects();
        }

        engine.destroy();
        clean_all_mesh_object();

        destroy_texture(&handle);
        descriptor.Destroy();
        descriptor_pool.destroy();
        vkDestroyPipelineLayout(handle.get_device(), pipelineLayout, nullptr);
        auto pipeline_map = VKDevice::get().get_pipeline_map();
        for (const auto &[key, value]: pipeline_map) {
            vkDestroyPipeline(handle.get_device(), value.pipeline, nullptr);
        }
        vkDestroyCommandPool(handle.get_device(), handle.get_command_pool(), nullptr);
        clean_all_shader_object();
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
    void init_need_objects() {
        while (true) {
            // 能编译过，但是漏洞百出 ，先预防一手，去制作一些日志
            auto render_data = vk_render_queue::instance().get_need_init();
            if (render_data.has_value()) {
                LOG_INFO(g_log(), "get {} from vk_render_queue", render_data.value()->debug_name);
                need_render_objects.push_back(render_data.value());
                find_graphics_shader_module(*handle_, render_data.value()->vertexPath_,
                                            render_data.value()->fragmentPath_,
                                            render_data.value()->geometryPath_);
                create_descriptor_set_layouts(*handle_, render_data.value()->vertexPath_,
                                              render_data.value()->fragmentPath_,
                                              render_data.value()->geometryPath_);
                create_mesh(*handle_, render_data.value(), VKDevice::get().get_mesh_map());

                // create_element_buffer(render_data.value()->indices_, &indices_map_);
                // create_texture(render_data.value()->textures, &texture_map_);
            } else {
                break;
            }
        }
    }


    void clean_all_mesh_object() {
        // 正式项目中，确保 vkDeviceWaitIdle 后按顺序销毁资源是专业开发者的标准做法
        for (const auto &[key, value]: VKDevice::get().get_mesh_map()) {
            vmaDestroyBuffer(handle_->get_allocator(), value.mesh.vertices_buffer, value.mesh.vBufferAllocation);
            // ->不清理会直接爆异常
        }
    }

    void clean_all_shader_object() {
        // 正式项目中，确保 vkDeviceWaitIdle 后按顺序销毁资源是专业开发者的标准做法
        for (const auto &[key, value]: VKDevice::get().get_shader_map()) {
            vkDestroyShaderModule(handle_->get_device(), value.shader, nullptr);
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
