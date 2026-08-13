//
// Created by 潘鑫 on 2026/7/22.
//

#ifndef HELLO_MAC_VULKAN_EXECUTE_COMMAND_H
#define HELLO_MAC_VULKAN_EXECUTE_COMMAND_H

#include <map>
#include <utility>
#include <vk_mem_alloc.h>
#include "APP_utility_mixins.h"


class command_submit_manager {
    static std::mutex submitMutex_;
    static std::vector<std::function<void(VkCommandBuffer commandBuffer)> > callback_functions_;

public:
    static auto &get_mutex() {
        return submitMutex_;
    }

    /**
     * 将之前汇总的函数全部提交
     * 问题是 这里是否 应是 update_descriptor
     */
    static void execute_callback_functions();

    static bool command_buffer_submit(uint32_t commandBufferCount,
                                      const VkCommandBuffer *pCommandBuffers,
                                      VkFence fence                                 = VK_NULL_HANDLE,
                                      const void *pNext                             = nullptr,
                                      uint32_t waitSemaphoreCount                   = 0,
                                      const VkSemaphore *pWaitSemaphores            = nullptr,
                                      const VkPipelineStageFlags *pWaitDstStageMask = nullptr,
                                      uint32_t signalSemaphoreCount                 = 0,
                                      const VkSemaphore *pSignalSemaphores          = nullptr
    );

    static void add_execute_function(const std::function<void(VkCommandBuffer commandBuffer)> &callback,
                                     VkFence fence = VK_NULL_HANDLE);


    static VkResult command_present(const VkPresentInfoKHR &presentInfo);
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

// 这里有两个不同的 add_execute_function , 一个是 统计之后再 上传的,另一个是 直接上传的


#endif //HELLO_MAC_VULKAN_EXECUTE_COMMAND_H
