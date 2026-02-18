//
// Created by 潘鑫 on 2026/2/14.
//
#include "engine.h"

#include <array>

#include "vulkan_device_handle.h"


#include "model_matrix.h"

const VkImage &VK_handle::get_current_swap_chain_image() {
    return get_swap_chain_images()[imageIndex];
}

const VkImageView &VK_handle::get_current_swap_image_view() {
    return get_swap_image_views()[imageIndex];
}

void VK_handle::create_command_buffer() {
    VkCommandBufferAllocateInfo cbAllocCI{
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = get_command_pool(),
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = maxFramesInFlight
    };
    VK_CHECK_RESULT_NOT_EXIT(vkAllocateCommandBuffers(get_device(), &cbAllocCI,
                                 command_buffers_.data()));
}

void VK_handle::create_shader_data_buffer() {
    // Shader data buffers
    for (auto i = 0; i < maxFramesInFlight; i++) {
        VkBufferCreateInfo uBufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size  = 32 * 1024, // 32K
            .usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
        };
        VmaAllocationCreateInfo uBufferAllocCI{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                     VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
                     VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };
        VK_CHECK_RESULT_NOT_EXIT(
                                 vmaCreateBuffer(get_allocator(),
                                     &uBufferCI,
                                     &uBufferAllocCI,
                                     &uniform_buffers_[i].
                                     buffer,
                                     &uniform_buffers_[i].allocation,
                                     nullptr));
    }
}


void VK_handle::create_fences() {
    VkFenceCreateInfo fenceCI{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    for (auto i = 0; i < maxFramesInFlight; i++) {
        VK_CHECK_RESULT_NOT_EXIT(vkCreateFence(get_device(), &fenceCI, nullptr, &fences_[i]));
    }
}


void VK_handle::create_present_Semaphores() {
    VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    for (auto i = 0; i < maxFramesInFlight; i++) {
        VK_CHECK_RESULT_NOT_EXIT(vkCreateSemaphore(get_device(), &semaphoreCI,
                                     nullptr, &present_semaphores_[i]));
    }
}


void VK_handle::create_renderSemaphores() {
    VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    render_to_image_semaphores_.resize(get_swap_image_views().size());
    LOG_INFO(g_log(), "get_swap_image_view size :  {}!", render_to_image_semaphores_.size());
    for (auto &semaphore: render_to_image_semaphores_) {
        VK_CHECK_RESULT_NOT_EXIT(vkCreateSemaphore(get_device(), &semaphoreCI, nullptr, &semaphore));
    }
}

void VK_handle::put_one_image_to_screen() {
    // Submit to graphics queue
    VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // 为了处理“交换链图像（Swapchain Image）还没准备好”的问题  图像还没有从显示器“拿回来”
    auto cb = get_current_command_buffer();
    VkSubmitInfo submitInfo{
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount   = 1,
        .pWaitSemaphores      = &get_current_presentSemaphores(),
        .pWaitDstStageMask    = &waitStages,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &cb,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = &get_can_render_to_image_semaphores()[imageIndex], // 不需要++ ？？可以，
    };
    VK_CHECK_RESULT_NOT_EXIT(vkQueueSubmit(get_queue(), 1, &submitInfo, get_current_fences()));

    frameIndex = (frameIndex + 1) % maxFramesInFlight;
    VkPresentInfoKHR presentInfo{
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &get_can_render_to_image_semaphores()[imageIndex], // 不需要++ ？？可以，
        .swapchainCount     = 1,
        .pSwapchains        = &get_swap_chain(),
        .pImageIndices      = &imageIndex
    };
    auto result = vkQueuePresentKHR(get_queue(), &presentInfo);
    if (result == VK_SUCCESS) {
    } else if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
               framebufferResized) {
        recreate_swap_chain();
        destroy_and_recreate_fence_and_semaphore();
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        std::cout << "failed to acquire swap chain image!" << std::endl;
    }
}


void VK_handle::get_one_image_can_render() {
    // forces the CPU to stop and wait until the GPU has finished executing a specific batch of commands
    VK_CHECK_RESULT_NOT_EXIT(vkWaitForFences(get_device(), 1, &get_current_fences(), true,
                                 UINT64_MAX));
    VK_CHECK_RESULT_NOT_EXIT(vkResetFences(get_device(), 1, &get_current_fences()));
    auto result = vkAcquireNextImageKHR(get_device(),
                                        get_swap_chain(),
                                        UINT64_MAX,
                                        get_current_presentSemaphores(),
                                        VK_NULL_HANDLE,
                                        &imageIndex);
    if (result == VK_SUCCESS) {
    } else if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
               framebufferResized) {
        recreate_swap_chain();
        destroy_and_recreate_fence_and_semaphore();
        VK_CHECK_RESULT_NOT_EXIT(vkWaitForFences(get_device(), 1, &get_current_fences(), true,
                                     UINT64_MAX));
        VK_CHECK_RESULT_NOT_EXIT(vkResetFences(get_device(), 1, &get_current_fences()));
        auto result = vkAcquireNextImageKHR(get_device(),
                                            get_swap_chain(),
                                            UINT64_MAX,
                                            get_current_presentSemaphores(),
                                            VK_NULL_HANDLE,
                                            &imageIndex);
        if (result == VK_SUCCESS) {
        } else {
            std::cout << "failed to acquire swap chain image after recreate!" << std::endl;
            exit(0);
        }
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        std::cout << "failed to acquire swap chain image!" << std::endl;
    }
}


void VK_handle::destroy_and_recreate_fence_and_semaphore() {
    for (auto i = 0; i < maxFramesInFlight; i++) {
        vkDestroyFence(get_device(), fences_[i], nullptr);                 //  这里还需要
        vkDestroySemaphore(get_device(), present_semaphores_[i], nullptr); //
    }
    for (auto i = 0; i < render_to_image_semaphores_.size(); i++) {
        vkDestroySemaphore(get_device(), render_to_image_semaphores_[i], nullptr);
    }
    create_fences();
    create_present_Semaphores();
    create_renderSemaphores();
    imageIndex = 0;
    frameIndex = 0;
}

void VK_handle::engine_destroy() {
    VK_CHECK_RESULT_NOT_EXIT(vkDeviceWaitIdle(get_device()));
    for (auto i = 0; i < maxFramesInFlight; i++) {
        vkDestroyFence(get_device(), fences_[i], nullptr);                 //  这里还需要
        vkDestroySemaphore(get_device(), present_semaphores_[i], nullptr); //
        vmaDestroyBuffer(get_allocator(), uniform_buffers_[i].buffer, uniform_buffers_[i].allocation);
    }
    for (auto i = 0; i < render_to_image_semaphores_.size(); i++) {
        vkDestroySemaphore(get_device(), render_to_image_semaphores_[i], nullptr);
    }
}


const uint32_t WIDTH  = 1280; // 也是需要更改的
const uint32_t HEIGHT = 720;

Point_3 camPos{1.0f, 2.0f, 6.0f};


ShaderData get_shader_data() {
    ShaderData shaderData;
    Quaternion r;
    perspective_matrix_4x4(reinterpret_cast<float *>(&shaderData.projection),
                           45.0f / 180.0f * std::acos(-1.0), (float) WIDTH / (float) HEIGHT, 0.1f, 32.0f);
    view_matrix_4x4(reinterpret_cast<float *>(&shaderData.view), camPos, r);
    for (auto i = 0; i < 3; i++) {
        Point_3 instancePos{(float) (i - 1) * 4.0f, 0.0f, 0.0f};
        auto point = reinterpret_cast<float *>(&shaderData.model[i]);
        scale s;
        model_matrix_4x4(point, instancePos, r, s);
    }
    return shaderData;
}


[[nodiscard]] void *uniform_buffer::get_point_mapped_address() const {
    const auto &handle = VK_handle::get();
    VmaAllocationInfo info;
    vmaGetAllocationInfo(handle.get_allocator(), allocation, &info);
    VkMemoryPropertyFlags props;
    vmaGetMemoryTypeProperties(handle.get_allocator(), info.memoryType, &props);
    const bool isVisible = props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    if (isVisible) {
        return info.pMappedData;
    }
    return nullptr;
}


[[nodiscard]] VkDeviceAddress uniform_buffer::get_gpu_device_address() const {
    const auto &handle = VK_handle::get();
    VkBufferDeviceAddressInfo uBufferBdaInfo{
        .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = buffer
    };
    auto deviceAddress = vkGetBufferDeviceAddress(handle.get_device(), &uBufferBdaInfo);
    return deviceAddress;
}
