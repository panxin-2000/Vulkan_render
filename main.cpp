/* Copyright (c) 2025-2026, Sascha Willems
 * SPDX-License-Identifier: MIT
 */

#define VK_NO_PROTOTYPES

#include <volk.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <array>
#include <string>
#include <iostream>
#include <fstream>
#include <vk_mem_alloc.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <ktx.h>
#include <ktxvulkan.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <thread>

uint32_t imageIndex{0};
uint32_t frameIndex{0};
bool updateSwapchain{false};


static inline void chk(VkResult result) {
    if (result != VK_SUCCESS) {
        std::cerr << "Vulkan call returned an error (" << result << ")\n";
        exit(result);
    }
}

static inline void chkSwapchain(VkResult result) {
    if (result < VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            updateSwapchain = true;
            return;
        }
        std::cerr << "Vulkan call returned an error (" << result << ")\n";
        exit(result);
    }
}

static inline void chk(bool result) {
    if (!result) {
        std::cerr << "Call returned an error\n";
        exit(result);
    }
}


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

VkCommandPool commandPool{VK_NULL_HANDLE};
VkPipeline pipeline{VK_NULL_HANDLE};
// VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};  // 直接注释后就能用，运气稍微有点好

std::array<VkCommandBuffer, maxFramesInFlight> commandBuffers;
std::array<VkFence, maxFramesInFlight> fences;
std::array<VkSemaphore, maxFramesInFlight> presentSemaphores;
std::vector<VkSemaphore> renderSemaphores;


struct ShaderData {
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model[3];
    glm::vec4 lightPos{0.0f, -10.0f, 10.0f, 0.0f};
    uint32_t selected{1};
} shaderData{};

struct ShaderDataBuffer {
    VmaAllocation allocation{VK_NULL_HANDLE};
    VkBuffer buffer{VK_NULL_HANDLE};
    VkDeviceAddress deviceAddress{};
    void *mapped{nullptr};
};

std::array<ShaderDataBuffer, maxFramesInFlight> shaderDataBuffers;


glm::vec3 camPos{0.0f, 0.0f, -6.0f};
glm::vec3 objectRotations[3]{};


int main(int argc, char *argv[]) {
    handle.create_instance();
    handle.choose_one_physical_device();
    handle.create_device();
    handle.create_VMA();
    handle.create_surface();
    handle.create_swap_chain();
    handle.create_swap_chain_image_view();
    handle.create_depth_image_view();

    // Window and surface
    Descriptor_Pool descriptor_pool(&handle, 250);
    descriptor_pool.init_Descriptor_Pool();
    Descriptor descriptor(&handle, &descriptor_pool);


    // Mesh data
    auto [vBuffer, vBufSize,indexCount] = create_mesh_data(handle, vBufferAllocation);


    // Shader data buffers
    for (auto i = 0; i < maxFramesInFlight; i++) {
        VkBufferCreateInfo uBufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = sizeof(ShaderData),
            .usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        };
        VmaAllocationCreateInfo uBufferAllocCI{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                     VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };
        chk(vmaCreateBuffer(handle.get_allocator(), &uBufferCI, &uBufferAllocCI, &shaderDataBuffers[i].buffer,
                            &shaderDataBuffers[i].allocation, nullptr));
        chk(vmaMapMemory(handle.get_allocator(), shaderDataBuffers[i].allocation, &shaderDataBuffers[i].mapped));
        VkBufferDeviceAddressInfo uBufferBdaInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = shaderDataBuffers[i].buffer
        };
        shaderDataBuffers[i].deviceAddress = vkGetBufferDeviceAddress(handle.get_device(), &uBufferBdaInfo);
    }
    // Sync objects
    VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fenceCI{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    for (auto i = 0; i < maxFramesInFlight; i++) {
        chk(vkCreateFence(handle.get_device(), &fenceCI, nullptr, &fences[i]));
        chk(vkCreateSemaphore(handle.get_device(), &semaphoreCI, nullptr, &presentSemaphores[i]));
    }
    renderSemaphores.resize(handle.get_swap_image_view().size());
    for (auto &semaphore: renderSemaphores) {
        chk(vkCreateSemaphore(handle.get_device(), &semaphoreCI, nullptr, &semaphore));
    }
    // Command pool
    VkCommandPoolCreateInfo commandPoolCI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = handle.get_queue_Family()
    };
    chk(vkCreateCommandPool(handle.get_device(), &commandPoolCI, nullptr, &commandPool));
    VkCommandBufferAllocateInfo cbAllocCI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .commandPool = commandPool,
        .commandBufferCount = maxFramesInFlight
    };
    chk(vkAllocateCommandBuffers(handle.get_device(), &cbAllocCI, commandBuffers.data()));

    // 目的是为了简化函数，
    // Texture images
    auto textureDescriptors = create_textures_to_gpu(&handle, commandPool);

    descriptor.CreateDescriptorSetLayout(textureDescriptors.size());
    descriptor.AllocateDescriptorSets(textureDescriptors.size());
    descriptor.update_descriptor_sets(textureDescriptors);
    // 到这里的时候贴图就更新完毕了
    auto pipelineLayout = descriptor.CreatePipelineLayout();
    auto shaderModule = createshaderModule(handle);
    auto shaderStages = createShaderStages(shaderModule);

    pipeline = create_pipeline(handle, shaderStages, pipelineLayout);
    // Render loop
    while (!glfwWindowShouldClose(handle.window_)) {
        glfwPollEvents();
        // Sync
        chk(vkWaitForFences(handle.get_device(), 1, &fences[frameIndex], true, UINT64_MAX));
        chk(vkResetFences(handle.get_device(), 1, &fences[frameIndex]));
        chkSwapchain(vkAcquireNextImageKHR(handle.get_device(), handle.get_swap_chain(), UINT64_MAX,
                                           presentSemaphores[frameIndex], VK_NULL_HANDLE,
                                           &imageIndex));


        // Update shader data
        shaderData.projection = glm::perspective(glm::radians(45.0f), (float) WIDTH / (float) HEIGHT, 0.1f, 32.0f);
        shaderData.view = glm::translate(glm::mat4(1.0f), camPos);
        for (auto i = 0; i < 3; i++) {
            auto instancePos = glm::vec3((float) (i - 1) * 3.0f, 0.0f, 0.0f);
            shaderData.model[i] = glm::translate(glm::mat4(1.0f), instancePos) * glm::mat4_cast(
                                      glm::quat(objectRotations[i]));
        }
        memcpy(shaderDataBuffers[frameIndex].mapped, &shaderData, sizeof(ShaderData));

        // Build command buffer
        auto cb = commandBuffers[frameIndex];
        chk(vkResetCommandBuffer(cb, 0));
        VkCommandBufferBeginInfo cbBI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };
        chk(vkBeginCommandBuffer(cb, &cbBI));
        std::array<VkImageMemoryBarrier2, 2> outputBarriers{
            VkImageMemoryBarrier2{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = 0,
                .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                .image = handle.get_swap_chain_images()[imageIndex],
                .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1}
            },
            VkImageMemoryBarrier2{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
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
            .imageView = handle.get_swap_image_view()[imageIndex],
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue{.color{0.0f, 0.0f, 0.0f, 1.0f}}
        };
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
                .extent{
                    .width = handle.get_surface_caps().currentExtent.width,
                    .height = handle.get_surface_caps().currentExtent.height
                }
            },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentInfo,
            .pDepthAttachment = &depthAttachmentInfo
        };
        vkCmdBeginRendering(cb, &renderingInfo);
        VkViewport vp{
            .width = static_cast<float>(handle.get_surface_caps().currentExtent.width),
            .height = static_cast<float>(handle.get_surface_caps().currentExtent.height), .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
        vkCmdSetViewport(cb, 0, 1, &vp);
        VkRect2D scissor{
            .extent{
                .width = handle.get_surface_caps().currentExtent.width,
                .height = handle.get_surface_caps().currentExtent.height
            }
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
                           &shaderDataBuffers[frameIndex].deviceAddress);
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
            .image = handle.get_swap_chain_images()[imageIndex],
            .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1}
        };
        VkDependencyInfo barrierPresentDependencyInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrierPresent
        };
        vkCmdPipelineBarrier2(cb, &barrierPresentDependencyInfo);
        chk(vkEndCommandBuffer(cb));


        // Submit to graphics queue
        VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        // 为了处理“交换链图像（Swapchain Image）还没准备好”的问题  图像还没有从显示器“拿回来”
        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &presentSemaphores[frameIndex],
            .pWaitDstStageMask = &waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = &cb,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &renderSemaphores[imageIndex], // 不需要++ ？？可以，
        };
        chk(vkQueueSubmit(handle.get_queue(), 1, &submitInfo, fences[frameIndex]));

        frameIndex = (frameIndex + 1) % maxFramesInFlight;
        VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &renderSemaphores[imageIndex], // 不需要++ ？？可以，
            .swapchainCount = 1,
            .pSwapchains = &handle.get_swap_chain(),
            .pImageIndices = &imageIndex
        };
        chkSwapchain(vkQueuePresentKHR(handle.get_queue(), &presentInfo));
        // Event polling
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    // Tear down
    chk(vkDeviceWaitIdle(handle.get_device()));
    for (auto i = 0; i < maxFramesInFlight; i++) {
        vkDestroyFence(handle.get_device(), fences[i], nullptr);
        vkDestroySemaphore(handle.get_device(), presentSemaphores[i], nullptr);
        vmaUnmapMemory(handle.get_allocator(), shaderDataBuffers[i].allocation);
        vmaDestroyBuffer(handle.get_allocator(), shaderDataBuffers[i].buffer, shaderDataBuffers[i].allocation);
    }
    for (auto i = 0; i < renderSemaphores.size(); i++) {
        vkDestroySemaphore(handle.get_device(), renderSemaphores[i], nullptr);
    }


    vmaDestroyBuffer(handle.get_allocator(), vBuffer, vBufferAllocation); // 暂时先不清理->不清理会直接爆异常
    destroy_texture(&handle);
    descriptor.Destroy();
    descriptor_pool.destroy();
    vkDestroyPipelineLayout(handle.get_device(), pipelineLayout, nullptr);
    vkDestroyPipeline(handle.get_device(), pipeline, nullptr);
    vkDestroyCommandPool(handle.get_device(), commandPool, nullptr);
    vkDestroyShaderModule(handle.get_device(), shaderModule, nullptr);

    handle.destroy();
}
