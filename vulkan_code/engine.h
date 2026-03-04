//
// Created by 潘鑫 on 2026/1/27.
//

#ifndef HELLO_MAC_ENGINE_H
#define HELLO_MAC_ENGINE_H
#include <list>

#include "vulkan_utility.h"
#include <string>
#include <vector>
#include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>

#include "model_matrix.h"


struct Engine {
    std::vector<VkSemaphore> render_to_image_semaphores_;
    std::array<VkCommandPool, maxFramesInFlight> command_pools_     = {};
    std::array<VkCommandBuffer, maxFramesInFlight> command_buffers_ = {};
    std::array<VkQueryPool, maxFramesInFlight> query_pools          = {};
    std::array<VkFence, maxFramesInFlight> fences_                  = {};
    std::array<VkSemaphore, maxFramesInFlight> present_semaphores_  = {};

    uint32_t frameIndex = 0;
    uint32_t imageIndex = 0;

public:
    std::array<VkFence, maxFramesInFlight> &get_fences() {
        return fences_;
    }

    VkFence &get_current_fences() {
        return get_fences()[frameIndex];
    }

    std::array<VkSemaphore, maxFramesInFlight> &get_presentSemaphores() {
        return present_semaphores_;
    }

    VkSemaphore &get_current_presentSemaphores() {
        return get_presentSemaphores()[frameIndex];
    }

    std::vector<VkSemaphore> &get_can_render_to_image_semaphores() {
        return render_to_image_semaphores_;
    }

    VkSemaphore &get_current_renderSemaphores() {
        return get_can_render_to_image_semaphores()[frameIndex];
    }


    std::array<VkCommandBuffer, maxFramesInFlight> &get_command_buffers() {
        return command_buffers_;
    }

    VkCommandBuffer &get_current_command_buffer() {
        return get_command_buffers()[frameIndex];
    }

    void create_command_buffer();

    void create_fences();

    void create_present_Semaphores();

    void create_renderSemaphores();


    void engine_init() {
        create_command_buffer();
        create_fences();
        create_present_Semaphores();
        create_renderSemaphores();
    }

    void engine_destroy();

    void destroy_and_recreate_fence_and_semaphore();
};


#endif //HELLO_MAC_ENGINE_H
