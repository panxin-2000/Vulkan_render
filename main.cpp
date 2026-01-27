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


VmaAllocation vBufferAllocation{VK_NULL_HANDLE};


VKDevice handle;

VkPipeline pipeline{VK_NULL_HANDLE};


glm::vec3 camPos{0.0f, 0.0f, -6.0f};
glm::vec3 objectRotations[3]{};


int main(int argc, char *argv[]) {
    handle.init_device_handle();

    // Window and surface
    Descriptor_Pool descriptor_pool(&handle, 250);
    descriptor_pool.init_Descriptor_Pool();
    Descriptor descriptor(&handle, &descriptor_pool);


    // Mesh data
    auto [vBuffer, vBufSize,indexCount] = create_mesh_data(handle, vBufferAllocation);

    Engine engine(&handle);
    engine.create_command_pool();
    engine.create_command_buffer();
    engine.create_shader_data_buffer();
    engine.create_fences();
    engine.create_present_Semaphores();
    engine.create_renderSemaphores();


    // 目的是为了简化函数，
    // Texture images
    auto textureDescriptors = create_textures_to_gpu(&handle, engine.get_command_pool());

    descriptor.CreateDescriptorSetLayout(textureDescriptors.size());
    descriptor.AllocateDescriptorSets(textureDescriptors.size());
    descriptor.update_descriptor_sets(textureDescriptors);
    // 到这里的时候贴图就更新完毕了
    auto pipelineLayout = descriptor.CreatePipelineLayout();
    auto shaderModule = create_shader_module(handle, "assets/shader.slang");
    auto shaderStages = createShaderStages(shaderModule);

    pipeline = create_pipeline(handle, shaderStages, pipelineLayout);
    // Render loop
    while (!glfwWindowShouldClose(handle.window_)) {
        glfwPollEvents();

        // Sync

        if (false == engine.get_one_image_can_render()) {
            continue;
        }
        // Update shader data
        {
            shaderData.projection = glm::perspective(glm::radians(45.0f), (float) WIDTH / (float) HEIGHT, 0.1f, 32.0f);
            shaderData.view = glm::translate(glm::mat4(1.0f), camPos);
            for (auto i = 0; i < 3; i++) {
                auto instancePos = glm::vec3((float) (i - 1) * 3.0f, 0.0f, 0.0f);
                shaderData.model[i] = glm::translate(glm::mat4(1.0f), instancePos) * glm::mat4_cast(
                                          glm::quat(objectRotations[i]));
            }
            memcpy(engine.get_current_shader_data_buffer().mapped, &shaderData, sizeof(ShaderData));
        }
        // Build command buffer
        {
            auto cb = engine.get_current_command_buffer();
            VK_CHECK_RESULT(vkResetCommandBuffer(cb, 0));
            VkCommandBufferBeginInfo cbBI{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
            };
            VK_CHECK_RESULT(vkBeginCommandBuffer(cb, &cbBI));
            std::array<VkImageMemoryBarrier2, 2> outputBarriers{
                VkImageMemoryBarrier2{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    .srcAccessMask = 0,
                    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                    .image = engine.get_current_swap_chain_image(),
                    .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1}
                },
                VkImageMemoryBarrier2{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                    .srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                    .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                    .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                    .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                    .image = handle.get_depth_image(),
                    .subresourceRange{
                        .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, .levelCount = 1,
                        .layerCount = 1
                    }
                }
            };
            VkDependencyInfo barrierDependencyInfo{
                .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 2,
                .pImageMemoryBarriers = outputBarriers.data()
            };
            vkCmdPipelineBarrier2(cb, &barrierDependencyInfo);
            VkRenderingAttachmentInfo colorAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = engine.get_current_swap_image_view(),
                .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue{.color{0.0f, 0.0f, 0.0f, 1.0f}}
            };
            auto temp_extent = handle.get_current_extent();
            VkRenderingAttachmentInfo depthAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = handle.get_depth_image_view(),
                .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .clearValue = {.depthStencil = {1.0f, 0}}
            };
            VkRenderingInfo renderingInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                .renderArea{
                    .extent = temp_extent,
                },
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &colorAttachmentInfo,
                .pDepthAttachment = &depthAttachmentInfo
            };
            vkCmdBeginRendering(cb, &renderingInfo);
            VkViewport vp{
                .width = static_cast<float>(temp_extent.width),
                .height = static_cast<float>(temp_extent.height),
                .minDepth = 0.0f,
                .maxDepth = 1.0f
            };
            vkCmdSetViewport(cb, 0, 1, &vp);
            VkRect2D scissor{
                .extent = temp_extent,
            };
            vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            vkCmdSetScissor(cb, 0, 1, &scissor);
            vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
                                    &descriptor.get_descriptor_set_texture(), 0,
                                    nullptr);
            VkDeviceSize vOffset{0};
            vkCmdBindVertexBuffers(cb, 0, 1, &vBuffer, &vOffset);
            vkCmdBindIndexBuffer(cb, vBuffer, vBufSize, VK_INDEX_TYPE_UINT16);
            vkCmdPushConstants(cb, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VkDeviceAddress),
                               &engine.get_current_shader_data_buffer().deviceAddress);
            vkCmdDrawIndexed(cb, indexCount, 3, 0, 0, 0);
            vkCmdEndRendering(cb);
            VkImageMemoryBarrier2 barrierPresent{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstAccessMask = 0,
                .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                .image = engine.get_current_swap_chain_image(),
                .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1}
            };
            VkDependencyInfo barrierPresentDependencyInfo{
                .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers = &barrierPresent
            };
            vkCmdPipelineBarrier2(cb, &barrierPresentDependencyInfo);
            VK_CHECK_RESULT(vkEndCommandBuffer(cb));
        }
        engine.put_one_image_to_screen();
        // Event polling
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    engine.destroy();

    vmaDestroyBuffer(handle.get_allocator(), vBuffer, vBufferAllocation); // 暂时先不清理->不清理会直接爆异常
    destroy_texture(&handle);
    descriptor.Destroy();
    descriptor_pool.destroy();
    vkDestroyPipelineLayout(handle.get_device(), pipelineLayout, nullptr);
    vkDestroyPipeline(handle.get_device(), pipeline, nullptr);
    vkDestroyCommandPool(handle.get_device(), engine.get_command_pool(), nullptr);
    vkDestroyShaderModule(handle.get_device(), shaderModule, nullptr);

    handle.destroy();
}
