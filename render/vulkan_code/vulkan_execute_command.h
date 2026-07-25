//
// Created by 潘鑫 on 2026/7/22.
//

#ifndef HELLO_MAC_VULKAN_EXECUTE_COMMAND_H
#define HELLO_MAC_VULKAN_EXECUTE_COMMAND_H

#include <map>
#include <utility>
#include <vk_mem_alloc.h>
#include "APP_utility_mixins.h"


class command_submit {
    static std::mutex submitMutex;

    VkResult result_;

public:
    command_submit() = delete;

    command_submit(uint32_t commandBufferCount,
                   const VkCommandBuffer *pCommandBuffers,
                   VkFence fence                                 = VK_NULL_HANDLE,
                   const void *pNext                             = nullptr,
                   uint32_t waitSemaphoreCount                   = 0,
                   const VkSemaphore *pWaitSemaphores            = nullptr,
                   const VkPipelineStageFlags *pWaitDstStageMask = nullptr,
                   uint32_t signalSemaphoreCount                 = 0,
                   const VkSemaphore *pSignalSemaphores          = nullptr
    );

    VkResult get_result() const {
        return result_;
    }


    command_submit(const VkPresentInfoKHR &presentInfo);
};


class temp_command_execute {
    VkCommandPool pool            = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkFence fence_                = VK_NULL_HANDLE;

public:
    temp_command_execute();

    void add_execute_function(const std::function<void(VkCommandBuffer commandBuffer)> &callback,
                              VkFence fence = VK_NULL_HANDLE);

    ~temp_command_execute();
};


#endif //HELLO_MAC_VULKAN_EXECUTE_COMMAND_H
