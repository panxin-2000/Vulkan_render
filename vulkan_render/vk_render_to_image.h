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

VkPipeline pipeline{VK_NULL_HANDLE};
VmaAllocation vBufferAllocation{VK_NULL_HANDLE};

glm::vec3 camPos{0.0f, 0.0f, -6.0f};
glm::vec3 objectRotations[3]{};


const uint32_t WIDTH  = 1280; // 也是需要更改的
const uint32_t HEIGHT = 720;

void update_shader_data(Engine &engine) {
    shaderData.projection = glm::perspective(glm::radians(45.0f), (float) WIDTH / (float) HEIGHT, 0.1f, 32.0f);
    shaderData.view       = glm::translate(glm::mat4(1.0f), camPos);
    for (auto i = 0; i < 3; i++) {
        auto instancePos    = glm::vec3((float) (i - 1) * 3.0f, 0.0f, 0.0f);
        shaderData.model[i] = glm::translate(glm::mat4(1.0f), instancePos) * glm::mat4_cast(
                                   glm::quat(objectRotations[i]));
    }
    memcpy(engine.get_current_shader_data_buffer().mapped, &shaderData, sizeof(ShaderData));
}

#include <map>

class vk_render_GPU {
    mutable std::mutex mtx;
    std::atomic<bool> have_object_need_update = false;
#define not_start 0
#define running 1
#define need_stop 2
    std::atomic<uint32_t> need_render = not_start; // 这里状态有点少了，需要 未开始，运行中，需停止

    std::map<logic_render_data *, shader_and_share> pipelineShaderStage_maps_;
    std::map<std::string, texture_and_share> texture_map_;
    std::map<Vertices_type, buffer_and_share> vertices_map_;
    std::map<Indices_type, buffer_and_share> indices_map_;

    VKDevice *handle_;

public:
    void render_thread(VKDevice &handle) {
        if (need_render == running) {
            handle_ = &handle;
            return; // 已经在运行中了，直接返回
        }
        need_render = running; // 设置为运行中
        Descriptor_Pool descriptor_pool(&handle, 250);
        descriptor_pool.init_Descriptor_Pool();
        Descriptor descriptor(&handle, &descriptor_pool);

        // Mesh data
        auto mesh = create_mesh_data(handle, vBufferAllocation);

        Engine engine(handle);
        engine.init();
        // 目的是为了简化函数，
        // Texture images
        auto textureDescriptors = create_textures_to_gpu(&handle, engine.get_command_pool());

        descriptor.CreateDescriptorSetLayout(textureDescriptors.size());
        descriptor.AllocateDescriptorSets(textureDescriptors.size());
        descriptor.update_descriptor_sets(textureDescriptors);
        // 到这里的时候贴图就更新完毕了
        auto pipelineLayout = descriptor.CreatePipelineLayout();
        // 有点难整理清楚

        auto shaderStages = create_shader_module(handle,
                                                 "/Users/panxin/CLionProjects/hello_mac/render/shader/temp.vert.spv",
                                                 "/Users/panxin/CLionProjects/hello_mac/render/shader/temp.frag.spv",
                                                 "");
        auto shaderStages_new = new decltype (shaderStages)(shaderStages);
        pipeline              = create_pipeline(handle, shaderStages, pipelineLayout);


        while (need_render == running) {
            {
                std::unique_lock<std::mutex> lock(mtx);
                init_need_objects(); // 主要是复制内存的操作
                update_need_objects();
            }
            engine.get_one_image_can_render();
            update_shader_data(engine); // 这里是一个需要同步的点
            build_command_buffer(engine, pipeline, pipelineLayout, descriptor, mesh);
            engine.put_one_image_to_screen();

            // render_object_function();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            clean_need_objects();
        }
        clean_all_object();

        engine.destroy();

        vmaDestroyBuffer(handle.get_allocator(), mesh.vertices_buffer, vBufferAllocation); // 暂时先不清理->不清理会直接爆异常
        destroy_texture(&handle);
        descriptor.Destroy();
        descriptor_pool.destroy();
        vkDestroyPipelineLayout(handle.get_device(), pipelineLayout, nullptr);
        vkDestroyPipeline(handle.get_device(), pipeline, nullptr);
        vkDestroyCommandPool(handle.get_device(), engine.get_command_pool(), nullptr);
        // vkDestroyShaderModule(handle.get_device(), shaderModule, nullptr);
        for (auto shaderStage: shaderStages) {
            vkDestroyShaderModule(handle.get_device(), shaderStage.module, nullptr);
        }
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

    void create_vertex_shader(logic_render_data *data,
                              std::map<logic_render_data *, shader_and_share> *map) {
        if (data != nullptr) {
            auto it = map->find(data);
            if (it != map->end()) {
                it->second.shared_number++;
            } else {
                auto shaderStages = create_shader_module(*handle_,
                                                         data->vertexPath_,
                                                         data->fragmentPath_,
                                                         data->geometryPath_);
                const auto shaderStages_new = new decltype (shaderStages)(shaderStages);
                map->insert({data, {shaderStages_new, 1}});
            }
        }
    }

private:
    void init_need_objects() {
        while (true) {
            auto render_data = vk_render::instance().get_need_init();
            if (render_data.has_value()) {
                create_vertex_shader(render_data.value(), &pipelineShaderStage_maps_);

                for (const auto &temp: render_data.value()->vertex_and_attributes_) {
                    create_vertex_buffer(temp.vertices_, temp.size, temp.data, &vertices_map_);
                }
                create_element_buffer(render_data.value()->indices_, &indices_map_);
                create_texture(render_data.value()->textures, &texture_map_);

                render_data.value()->fragmentPath_;
            } else {
                break;
            }
        }
    }


    void clean_all_object() {
        // 正式项目中，确保 vkDeviceWaitIdle 后按顺序销毁资源是专业开发者的标准做法
    }

    void update_need_objects() {
        // 内存内容的更新
        // 先查找放置在哪里来
        // 之后再更新数据
    }

    void clean_need_objects() {
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
