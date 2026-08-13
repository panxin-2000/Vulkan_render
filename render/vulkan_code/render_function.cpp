//
// Created by 潘鑫 on 2026/3/4.
//

#include "../engine.h"

#include "vulkan_backend.h"
#include "vulkan_buffer.h"
#include "vulkan_execute_command.h"

const VkImage &Engine::get_current_swap_chain_image(uint index) const {
    return get_swap_chain_images()[get_imageIndex()]->get_image_handle();
}

const VkImageView &Engine::get_current_swap_image_view(uint index) const {
    return get_swap_chain_images()[get_imageIndex()]->get_image_view();
}

const VkImage &Engine::get_current_depth_image(uint index) const {
    return get_depth_images()[get_imageIndex()]->get_image_handle();
}

const VKR_image_ptr &Engine::get_current_depth_image_ptr(uint index) const {
    return get_depth_images()[get_imageIndex()];
}

const VkImageView &Engine::get_current_depth_view(uint index) const {
    return get_depth_images()[get_imageIndex()]->get_image_view();
}

const VKR_image_ptr &Engine::get_current_position_image_ptr(uint index) const {
    return G_buffer_Position_images_[0];
}

const VKR_image_ptr &Engine::get_current_normal_image_ptr(uint index) const {
    return g_buffer_Normal_images_[0];
}

const VKR_image_ptr &Engine::get_current_baseColor_image_ptr(uint index) const {
    return G_buffer_BaseColor_images_[0];
}

const VkImage &Engine::get_current_position_image(uint index) const {
    return G_buffer_Position_images_[0]->get_image_handle();
}

const VkImageView &Engine::get_current_position_view(const uint index) const {
    assert(G_buffer_Position_images_.size() > index);
    return G_buffer_Position_images_.at(index)->get_image_view();
}

const VkImage &Engine::get_current_normal_image(uint index) const {
    return g_buffer_Normal_images_[0]->get_image_handle();
}

const VkImageView &Engine::get_current_normal_view(uint index) const {
    return g_buffer_Normal_images_[0]->get_image_view();
}

const VkImage &Engine::get_current_baseColor_image(uint index) const {
    return G_buffer_BaseColor_images_[0]->get_image_handle();
}

const VkImageView &Engine::get_current_baseColor_view(uint index) const {
    return G_buffer_BaseColor_images_[0]->get_image_view();
}

void Engine::submit_render_queue(uint64_t time_line) {
    // Submit to graphics queue
    VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // 为了处理“交换链图像（Swapchain Image）还没准备好”的问题  图像还没有从显示器“拿回来”
    auto cb = get_current_command_buffer();

    // uint32_t wait_semaphore_len = submit_task->wait_semaphore == VK_NULL_HANDLE ? 0 : 1;
    uint32_t signal_semaphore_len    = 2;
    VkSemaphore signal_semaphores[2] = {
        vk_timeline_semaphore_,
        get_can_render_to_image_semaphores()[get_imageIndex()]
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

    command_submit_manager::command_buffer_submit(1, &cb,
                                                  get_current_fences(),
                                                  &timeline_semaphore_submit_info,
                                                  1,
                                                  &get_current_presentSemaphores(),
                                                  &waitStages,
                                                  2,
                                                  signal_semaphores);
}


void Engine::copy_image_to_screen() {
    frameIndex = (frameIndex + 1) % maxFramesInFlight;
    VkPresentInfoKHR presentInfo{
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &get_can_render_to_image_semaphores()[imageIndex], // 不需要++ ？？可以，
        .swapchainCount     = 1,
        .pSwapchains        = &VK_backend::instance().get_swap_chain(),
        .pImageIndices      = &imageIndex
    }; {
        const auto result = command_submit_manager::command_present(presentInfo);
        if (result == VK_SUCCESS) {
        } else if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
                   VK_backend::instance().is_frame_buffer_resize()) {
            recreate_swap_chain();
            destroy_and_recreate_fence_and_semaphore();
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            std::cout << "failed to acquire swap chain image!" << std::endl;
        }
    }
}


void Engine::get_image_to_render() {
    // forces the CPU to stop and wait until the GPU has finished executing a specific batch of commands
    VK_CHECK_RESULT_NOT_EXIT(vkWaitForFences(VK_backend::instance().get_device(), 1, &get_current_fences(), true,
                                 UINT64_MAX));
    VK_CHECK_RESULT_NOT_EXIT(vkResetFences(VK_backend::instance().get_device(), 1, &get_current_fences()));
    auto result = vkAcquireNextImageKHR(VK_backend::instance().get_device(),
                                        VK_backend::instance().get_swap_chain(),
                                        UINT64_MAX,
                                        get_current_presentSemaphores(),
                                        VK_NULL_HANDLE,
                                        &imageIndex); // 其实是在这里执行了 ++ 的工作
    if (result == VK_SUCCESS) {
    } else if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
               VK_backend::instance().is_frame_buffer_resize()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        recreate_swap_chain();
        destroy_and_recreate_fence_and_semaphore();
        VK_CHECK_RESULT_NOT_EXIT(vkWaitForFences(VK_backend::instance().get_device(), 1, &get_current_fences(), true
                                    ,
                                     UINT64_MAX));
        VK_CHECK_RESULT_NOT_EXIT(vkResetFences(VK_backend::instance().get_device(), 1, &get_current_fences()));
        result = vkAcquireNextImageKHR(VK_backend::instance().get_device(),
                                       VK_backend::instance().get_swap_chain(),
                                       UINT64_MAX,
                                       get_current_presentSemaphores(),
                                       VK_NULL_HANDLE,
                                       &imageIndex);
        if (result == VK_SUCCESS) {
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // 有时候成功，有时候不能一次成功，不知道为什么
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        std::cout << "failed to acquire swap chain image!" << std::endl;
    }
}


void Engine::create_timeline_Semaphores() {
    VkSemaphoreTypeCreateInfo vk_semaphore_type_create_info = {
        VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO, nullptr, VK_SEMAPHORE_TYPE_TIMELINE, 0
    };
    VkSemaphoreCreateInfo vk_semaphore_create_info = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, &vk_semaphore_type_create_info, 0
    };
    vkCreateSemaphore(VK_backend::instance().get_device(), &vk_semaphore_create_info, nullptr, &vk_timeline_semaphore_);
}
