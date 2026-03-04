//
// Created by 潘鑫 on 2026/2/14.
//
#include "engine.h"

#include "vulkan_device_handle.h"


void Engine::create_command_buffer() {
    const auto &handle = VK_handle::get();

    VkCommandBufferAllocateInfo cbAllocCI{
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = handle.get_command_pool(),
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = maxFramesInFlight
    };
    VK_CHECK_RESULT_NOT_EXIT(vkAllocateCommandBuffers(handle.get_device(), &cbAllocCI,
                                 command_buffers_.data()));
}

void Engine::destroy_command_buffer() {
    const auto &handle = VK_handle::get();
    for (auto i = 0; i < maxFramesInFlight; i++) {
        vkFreeCommandBuffers(handle.get_device(), handle.get_command_pool(), 1, &command_buffers_[i]);
        command_buffers_[i] = VK_NULL_HANDLE;
    }
}


void Engine::create_fences() {
    const auto &handle = VK_handle::get();
    VkFenceCreateInfo fenceCI{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    for (auto i = 0; i < maxFramesInFlight; i++) {
        VK_CHECK_RESULT_NOT_EXIT(vkCreateFence(handle.get_device(), &fenceCI, nullptr, &fences_[i]));
    }
}

void Engine::destroy_fences() {
    const auto &handle = VK_handle::get();
    for (auto i = 0; i < maxFramesInFlight; i++) {
        vkDestroyFence(handle.get_device(), fences_[i], nullptr); //  这里还需要
        fences_[i] = VK_NULL_HANDLE;
    }
}

void Engine::create_present_Semaphores() {
    const auto &handle = VK_handle::get();
    VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    for (auto i = 0; i < maxFramesInFlight; i++) {
        VK_CHECK_RESULT_NOT_EXIT(vkCreateSemaphore(handle.get_device(), &semaphoreCI,
                                     nullptr, &present_semaphores_[i]));
    }
}

void Engine::destroy_present_Semaphores() {
    const auto &handle = VK_handle::get();
    for (auto i = 0; i < maxFramesInFlight; i++) {
        vkDestroySemaphore(handle.get_device(), present_semaphores_[i], nullptr); //
        present_semaphores_[i] = VK_NULL_HANDLE;
    }
}


void Engine::create_renderSemaphores() {
    const auto &handle = VK_handle::get();
    VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    render_to_image_semaphores_.resize(handle.get_swap_chain_images().size());
    LOG_INFO(g_log(), "get_swap_image_view size :  {}!", render_to_image_semaphores_.size());
    for (auto &semaphore: render_to_image_semaphores_) {
        VK_CHECK_RESULT_NOT_EXIT(vkCreateSemaphore(handle.get_device(), &semaphoreCI, nullptr, &semaphore));
    }
}

void Engine::destroy_renderSemaphores() {
    const auto &handle = VK_handle::get();
    for (auto i = 0; i < render_to_image_semaphores_.size(); i++) {
        vkDestroySemaphore(handle.get_device(), render_to_image_semaphores_[i], nullptr);
        render_to_image_semaphores_[i] = VK_NULL_HANDLE;
    }
}


void Engine::destroy_and_recreate_fence_and_semaphore() {
    destroy_fences();
    destroy_present_Semaphores();
    destroy_renderSemaphores();
    create_fences();
    create_present_Semaphores();
    create_renderSemaphores();
    imageIndex = 0;
    frameIndex = 0;
}

void Engine::engine_destroy() {
    const auto &handle = VK_handle::get();
    VK_CHECK_RESULT_NOT_EXIT(vkDeviceWaitIdle(handle.get_device()));
    destroy_fences();
    destroy_present_Semaphores();
    destroy_renderSemaphores();
    destroy_command_buffer();
}
