//
// Created by 潘鑫 on 2026/3/4.
//

#include "engine.h"

#include "vulkan_backend.h"
#include "vulkan_buffer.h"

const VkImage &VK_backend::get_current_swap_chain_image() const {
    return get_swap_chain_images()[engine_.imageIndex]->get_image_handle();
}

const VkImageView &VK_backend::get_current_swap_image_view() const {
    return get_swap_chain_images()[engine_.imageIndex]->get_image_view();
}

const VkImage &VK_backend::get_current_depth_image() const {
    return get_depth_images()[engine_.imageIndex]->get_image_handle();
}

const VkImageView &VK_backend::get_current_depth_view() const {
    return get_depth_images()[engine_.imageIndex]->get_image_view();
}

const VkImage &VK_backend::get_current_position_image() const {
    return G_buffer_Position_images_[0]->get_image_handle();
}

const VkImageView &VK_backend::get_current_position_view() const {
    return G_buffer_Position_images_[0]->get_image_view();
}

const VkImage &VK_backend::get_current_normal_image() const {
    return g_buffer_Normal_images_[0]->get_image_handle();
}

const VkImageView &VK_backend::get_current_normal_view() const {
    return g_buffer_Normal_images_[0]->get_image_view();
}

const VkImage &VK_backend::get_current_baseColor_image() const {
    return G_buffer_BaseColor_images_[0]->get_image_handle();
}

const VkImageView &VK_backend::get_current_baseColor_view() const {
    return G_buffer_BaseColor_images_[0]->get_image_view();
}

void VK_backend::submit_render_queue(uint64_t time_line) {
    // Submit to graphics queue
    VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // 为了处理“交换链图像（Swapchain Image）还没准备好”的问题  图像还没有从显示器“拿回来”
    auto cb = engine_.get_current_command_buffer();

    // uint32_t wait_semaphore_len = submit_task->wait_semaphore == VK_NULL_HANDLE ? 0 : 1;
    uint32_t signal_semaphore_len    = 2;
    VkSemaphore signal_semaphores[2] = {
        vk_timeline_semaphore_,
        engine_.get_can_render_to_image_semaphores()[engine_.imageIndex]
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
        .pWaitSemaphores      = &engine_.get_current_presentSemaphores(),
        .pWaitDstStageMask    = &waitStages,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &cb,
        .signalSemaphoreCount = 2,
        .pSignalSemaphores    = signal_semaphores, //  &get_can_render_to_image_semaphores()[imageIndex], // 不需要++ ？？可以，
    }; {
        std::lock_guard<std::mutex> lock(get_vkQueueSubmit_mutex());
        VK_CHECK_RESULT_NOT_EXIT(vkQueueSubmit(get_queue(), 1, &submitInfo, engine_.get_current_fences()));
    }
}


void VK_backend::copy_image_to_screen() {
    engine_.frameIndex = (engine_.frameIndex + 1) % maxFramesInFlight;
    VkPresentInfoKHR presentInfo{
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &engine_.get_can_render_to_image_semaphores()[engine_.imageIndex], // 不需要++ ？？可以，
        .swapchainCount     = 1,
        .pSwapchains        = &get_swap_chain(),
        .pImageIndices      = &engine_.imageIndex
    }; {
        std::lock_guard<std::mutex> lock(get_vkQueueSubmit_mutex());
        auto result = vkQueuePresentKHR(get_queue(), &presentInfo);
        if (result == VK_SUCCESS) {
        } else if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
                   framebufferResized) {
            recreate_swap_chain();
            engine_.destroy_and_recreate_fence_and_semaphore();
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            std::cout << "failed to acquire swap chain image!" << std::endl;
        }
    }
}


void VK_backend::get_image_to_render() {
    // forces the CPU to stop and wait until the GPU has finished executing a specific batch of commands
    VK_CHECK_RESULT_NOT_EXIT(vkWaitForFences(get_device(), 1, &engine_.get_current_fences(), true,
                                 UINT64_MAX));
    VK_CHECK_RESULT_NOT_EXIT(vkResetFences(get_device(), 1, &engine_.get_current_fences()));
    auto result = vkAcquireNextImageKHR(get_device(),
                                        get_swap_chain(),
                                        UINT64_MAX,
                                        engine_.get_current_presentSemaphores(),
                                        VK_NULL_HANDLE,
                                        &engine_.imageIndex); // 其实是在这里执行了 ++ 的工作
    if (result == VK_SUCCESS) {
    } else if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
               framebufferResized) {
        recreate_swap_chain();
        engine_.destroy_and_recreate_fence_and_semaphore();
        VK_CHECK_RESULT_NOT_EXIT(vkWaitForFences(get_device(), 1, &engine_.get_current_fences(), true,
                                     UINT64_MAX));
        VK_CHECK_RESULT_NOT_EXIT(vkResetFences(get_device(), 1, &engine_.get_current_fences()));
        auto result = vkAcquireNextImageKHR(get_device(),
                                            get_swap_chain(),
                                            UINT64_MAX,
                                            engine_.get_current_presentSemaphores(),
                                            VK_NULL_HANDLE,
                                            &engine_.imageIndex);
        if (result == VK_SUCCESS) {
        } else {
            std::cout << "failed to acquire swap chain image after recreate!" << std::endl;
            exit(0);
        }
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        std::cout << "failed to acquire swap chain image!" << std::endl;
    }
}


void VK_backend::create_timeline_Semaphores() {
    VkSemaphoreTypeCreateInfo vk_semaphore_type_create_info = {
        VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO, nullptr, VK_SEMAPHORE_TYPE_TIMELINE, 0
    };
    VkSemaphoreCreateInfo vk_semaphore_create_info = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, &vk_semaphore_type_create_info, 0
    };
    vkCreateSemaphore(get_device(), &vk_semaphore_create_info, nullptr, &vk_timeline_semaphore_);
}
