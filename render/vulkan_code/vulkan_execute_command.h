//
// Created by 潘鑫 on 2026/7/22.
//

#ifndef HELLO_MAC_VULKAN_EXECUTE_COMMAND_H
#define HELLO_MAC_VULKAN_EXECUTE_COMMAND_H

#include <map>
#include <vk_mem_alloc.h>


class Command_submit_manager {
    static std::mutex submitMutex_;
    static std::mutex callbackMutex_;
    static std::vector<std::function<void(VkCommandBuffer commandBuffer, uint64_t time_line)> > callback_functions_;
    VkCommandPool pool                              = VK_NULL_HANDLE;
    VkFence fence_                                  = VK_NULL_HANDLE;
    std::array<VkCommandBuffer, 2> command_buffers_ = {};
    std::atomic<uint64_t> command_buffer_count_     = 0;

public:
    static std::mutex &get_mutex();


    void create();

    /**
     * 将之前汇总的函数全部提交
     * 问题是 这里是否 应是 update_descriptor
     */
    void execute_callback_functions(const uint64_t time_line);

    void destroy();

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

    static void add_execute_function(
        const std::function<void(VkCommandBuffer commandBuffer, uint64_t time_line)> &callback,
        VkFence fence = VK_NULL_HANDLE);


    static VkResult command_copy_image_to_screen(const VkPresentInfoKHR &presentInfo);
};


#endif //HELLO_MAC_VULKAN_EXECUTE_COMMAND_H
