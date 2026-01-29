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

VkPipeline pipeline{VK_NULL_HANDLE};
VmaAllocation vBufferAllocation{VK_NULL_HANDLE};

glm::vec3 camPos{0.0f, 0.0f, -6.0f};
glm::vec3 objectRotations[3]{};


const uint32_t WIDTH = 1280; // 也是需要更改的
const uint32_t HEIGHT = 720;

void update_shader_data(Engine &engine) {
    shaderData.projection = glm::perspective(glm::radians(45.0f), (float) WIDTH / (float) HEIGHT, 0.1f, 32.0f);
    shaderData.view = glm::translate(glm::mat4(1.0f), camPos);
    for (auto i = 0; i < 3; i++) {
        auto instancePos = glm::vec3((float) (i - 1) * 3.0f, 0.0f, 0.0f);
        shaderData.model[i] = glm::translate(glm::mat4(1.0f), instancePos) * glm::mat4_cast(
                                  glm::quat(objectRotations[i]));
    }
    memcpy(engine.get_current_shader_data_buffer().mapped, &shaderData, sizeof(ShaderData));
}

class vk_render_GPU {
    mutable std::mutex mtx;
    std::atomic<bool> have_object_need_update = false;
    std::atomic<bool> need_render = true;

public:
    void render_thread(VKDevice &handle) {
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

        pipeline = create_pipeline(handle, shaderStages, pipelineLayout);


        while (need_render) {
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
        need_render = true;
    }

    void render_thread_stop() {
        need_render = false;
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
    void init_need_objects() {
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

    ~vk_render_GPU() {
    }

    vk_render_GPU(const vk_render_GPU &) = delete;

    vk_render_GPU &operator=(const vk_render_GPU &) = delete;

    vk_render_GPU(vk_render_GPU &&) = delete;

    vk_render_GPU &operator=(vk_render_GPU &&) = delete;
};


#endif //HELLO_MAC_VK_RENDER_TO_IMAGE_H
