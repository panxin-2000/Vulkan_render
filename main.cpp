/* Copyright (c) 2025-2026, Sascha Willems
 * SPDX-License-Identifier: MIT
 */

#define VK_NO_PROTOTYPES
#include <volk.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vk_mem_alloc.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <thread>

#include "engine.h"


const uint32_t WIDTH = 1280;
const uint32_t HEIGHT = 720;

#include "vulkan_device_handle.h"
#include "transfer_texture_to_gpu.h"
#include "descriptor_pool.h"
#include "descriptor.h"
#include "create_shader.h"
#include "create_pipeline.h"
#include "vertex_and_buffer_index.h"
#include "vulkan_build_command_buffer.h"


VmaAllocation vBufferAllocation{VK_NULL_HANDLE};


VKDevice handle;

VkPipeline pipeline{VK_NULL_HANDLE};


glm::vec3 camPos{0.0f, 0.0f, -6.0f};
glm::vec3 objectRotations[3]{};


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


int main(int argc, char *argv[]) {
    handle.init_device_handle();
    // Window and surface

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
    // auto shaderModule = create_shader_module(handle, "assets/shader.slang");


    // auto shaderStages = createShaderStages(shaderModule);
    auto shaderStages = create_shader_module(handle,
                                             "/Users/panxin/CLionProjects/hello_mac/render/shader/temp.vert.spv",
                                             "/Users/panxin/CLionProjects/hello_mac/render/shader/temp.frag.spv",
                                             "");

    pipeline = create_pipeline(handle, shaderStages, pipelineLayout);
    // Render loop
    while (!glfwWindowShouldClose(handle.window_)) {
        glfwPollEvents();

        engine.get_one_image_can_render();
        update_shader_data(engine);
        build_command_buffer(engine, pipeline, pipelineLayout, descriptor, mesh);
        engine.put_one_image_to_screen();
        // Event polling
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

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

    handle.destroy();
}
