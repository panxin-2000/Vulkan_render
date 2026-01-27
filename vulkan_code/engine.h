//
// Created by 潘鑫 on 2026/1/27.
//

#ifndef HELLO_MAC_ENGINE_H
#define HELLO_MAC_ENGINE_H


#include "vulkan_device_handle.h"

#include "descriptor_pool.h"


class Engine {
    VKDevice *handle_;
    VkCommandPool commandPool{VK_NULL_HANDLE};
    std::array<VkCommandBuffer, maxFramesInFlight> commandBuffers;

public:
    Engine(VKDevice *handle) : handle_{handle} {
    }

    void create_command_pool() {
        // Command pool
        VkCommandPoolCreateInfo commandPoolCI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = handle_->get_queue_Family()
        };
        VK_CHECK_RESULT(vkCreateCommandPool(handle_->get_device(), &commandPoolCI, nullptr, &commandPool));
    }

    void create_command_buffer() {
        VkCommandBufferAllocateInfo cbAllocCI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = maxFramesInFlight
        };
        VK_CHECK_RESULT(vkAllocateCommandBuffers(handle_->get_device(), &cbAllocCI, commandBuffers.data()));
    }

    VkCommandPool &get_command_pool() {
        return commandPool;
    }

    std::array<VkCommandBuffer, maxFramesInFlight> &get_command_buffers() {
        return commandBuffers;
    }
};


#endif //HELLO_MAC_ENGINE_H
