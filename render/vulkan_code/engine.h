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

#include "descriptor.h"
#include "descriptor_pool.h"
#include "model_matrix.h"


struct Engine {
    std::vector<VkCommandPool> command_pools_                        = {};
    std::vector<VkSemaphore> render_to_image_semaphores_            = {};
    std::array<VkCommandBuffer, maxFramesInFlight> command_buffers_ = {};
    std::array<VkQueryPool, maxFramesInFlight> query_pools          = {};
    std::array<VkFence, maxFramesInFlight> fences_                  = {};
    std::array<VkSemaphore, maxFramesInFlight> present_semaphores_  = {};
    std::vector<DescriptorSet_ptr> bindless_descriptor_sets_        = {};
    std::vector<DescriptorSet_ptr> global_descriptor_sets_          = {};
    std::vector<VkDescriptorPool> descriptor_pools                    = {};


    uint32_t frameIndex = 0;
    uint32_t imageIndex = 0;

public:
    std::array<VkFence, maxFramesInFlight> &get_fences() {
        return fences_;
    }

    const VkCommandPool &get_command_pool(const uint index = 0) const {
        return command_pools_.at(index);
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

    VkQueryPool &get_current_query_pool() {
        return query_pools[frameIndex];
    }

    void get_query_results();

    void create_command_buffer();

    void destroy_command_buffer();

    void create_query_pool();

    void destroy_query_pool();

    void create_fences();

    void destroy_fences();

    void create_present_Semaphores();

    void destroy_present_Semaphores();

    void create_renderSemaphores();

    void destroy_renderSemaphores();

    void create_command_pool();

    void destroy_command_pool();

    std::vector<DescriptorSet_ptr> allocate_global_descriptor_sets(const std::string &one_binding_name);

    std::vector<DescriptorSet_ptr> allocate_bindless_descriptor_sets(const std::string &one_binding_name);

    std::vector<DescriptorSet_ptr> get_bindless_descriptor_set(const uint index = 0);

    std::vector<DescriptorSet_ptr> get_global_descriptor_set(const uint index = 0);


    VkDescriptorPool get_descriptor_pool(const uint index = 0) const {
        return descriptor_pools.at(index);
    }

    void engine_init() {
        create_command_pool();
        create_command_buffer();
        create_fences();
        create_present_Semaphores();
        create_renderSemaphores();
        descriptor_pools.resize(1,VK_NULL_HANDLE);
        descriptor_pools.at(0) = init_current_descriptor_pool();

        bindless_descriptor_sets_ = allocate_bindless_descriptor_sets("");
        global_descriptor_sets_   = allocate_global_descriptor_sets("");
    }

    void engine_destroy();

    void destroy_and_recreate_fence_and_semaphore();
};


#endif //HELLO_MAC_ENGINE_H
