//
// Created by 潘鑫 on 2026/2/14.
//
#include "engine.h"

#include <array>

#include "vulkan_device_handle.h"


#include "model_matrix.h"

const VkImage &VK_handle::get_current_swap_chain_image() const {
    return get_swap_chain_images()[imageIndex]->get_image_handle();
}

const VkImageView &VK_handle::get_current_swap_image_view() const {
    return get_swap_chain_images()[imageIndex]->get_image_view();
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
    render_to_image_semaphores_.resize(get_swap_chain_images().size());
    LOG_INFO(g_log(), "get_swap_image_view size :  {}!", render_to_image_semaphores_.size());
    for (auto &semaphore: render_to_image_semaphores_) {
        VK_CHECK_RESULT_NOT_EXIT(vkCreateSemaphore(get_device(), &semaphoreCI, nullptr, &semaphore));
    }
}

void VK_handle::create_timeline_Semaphores() {
    VkSemaphoreTypeCreateInfo vk_semaphore_type_create_info = {
        VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO, nullptr, VK_SEMAPHORE_TYPE_TIMELINE, 0
    };
    VkSemaphoreCreateInfo vk_semaphore_create_info = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, &vk_semaphore_type_create_info, 0
    };
    vkCreateSemaphore(get_device(), &vk_semaphore_create_info, nullptr, &vk_timeline_semaphore_);
}

void VK_handle::submit_render_queue(uint64_t time_line) {
    // Submit to graphics queue
    VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // 为了处理“交换链图像（Swapchain Image）还没准备好”的问题  图像还没有从显示器“拿回来”
    auto cb = get_current_command_buffer();

    // uint32_t wait_semaphore_len = submit_task->wait_semaphore == VK_NULL_HANDLE ? 0 : 1;
    uint32_t signal_semaphore_len    = 2;
    VkSemaphore signal_semaphores[2] = {
        vk_timeline_semaphore_,
        get_can_render_to_image_semaphores()[imageIndex]
    };
    uint64_t signal_semaphore_values[2] = {time_line, 0};

    VkTimelineSemaphoreSubmitInfo timeline_semaphore_submit_info = {
        VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
        nullptr,
        0,
        nullptr,
        signal_semaphore_len,
        signal_semaphore_values
    };

    VkSubmitInfo submitInfo{
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext                = &timeline_semaphore_submit_info,
        .waitSemaphoreCount   = 1,
        .pWaitSemaphores      = &get_current_presentSemaphores(),
        .pWaitDstStageMask    = &waitStages,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &cb,
        .signalSemaphoreCount = 2,
        .pSignalSemaphores    = signal_semaphores, //  &get_can_render_to_image_semaphores()[imageIndex], // 不需要++ ？？可以，
    };
    VK_CHECK_RESULT_NOT_EXIT(vkQueueSubmit(get_queue(), 1, &submitInfo, get_current_fences()));
}

void VK_handle::copy_image_to_screen() {
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


void VK_handle::get_image_to_render() {
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
        vkDestroyFence(get_device(), fences_[i], nullptr); //  这里还需要
        fences_[i] = VK_NULL_HANDLE;
        vkDestroySemaphore(get_device(), present_semaphores_[i], nullptr); //
        present_semaphores_[i] = VK_NULL_HANDLE;
    }
    for (auto i = 0; i < render_to_image_semaphores_.size(); i++) {
        vkDestroySemaphore(get_device(), render_to_image_semaphores_[i], nullptr);
        render_to_image_semaphores_[i] = VK_NULL_HANDLE;
    }

    vkDestroySemaphore(get_device(), vk_timeline_semaphore_, nullptr);
    vk_timeline_semaphore_ = VK_NULL_HANDLE; // 这里设置为 VK_NULL_HANDLE 了，但是上面几个并没有
}





