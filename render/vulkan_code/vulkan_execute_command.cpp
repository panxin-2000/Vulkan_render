//
// Created by 潘鑫 on 2026/7/22.
//

#include "vulkan_execute_command.h"
#include "vulkan_backend.h"
#include "vulkan_buffer.h"

std::mutex command_submit::submitMutex;

command_submit::command_submit(const VkPresentInfoKHR &presentInfo) {
    const auto &backend = VK_backend::instance();
    std::lock_guard<std::mutex> lock(submitMutex);
    result_ = vkQueuePresentKHR(backend.get_queue(), &presentInfo);
}

command_submit::command_submit(const uint32_t commandBufferCount,
                               const VkCommandBuffer *pCommandBuffers,
                               const VkFence fence,
                               const void *pNext,
                               const uint32_t waitSemaphoreCount,
                               const VkSemaphore *pWaitSemaphores,
                               const VkPipelineStageFlags *pWaitDstStageMask,
                               const uint32_t signalSemaphoreCount,
                               const VkSemaphore *pSignalSemaphores) {
    const auto &backend = VK_backend::instance();
    const VkSubmitInfo submitInfo{
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext                = pNext,
        .waitSemaphoreCount   = waitSemaphoreCount,
        .pWaitSemaphores      = pWaitSemaphores,
        .pWaitDstStageMask    = pWaitDstStageMask,
        .commandBufferCount   = commandBufferCount,
        .pCommandBuffers      = pCommandBuffers,
        .signalSemaphoreCount = signalSemaphoreCount,
        .pSignalSemaphores    = pSignalSemaphores,
    };
    std::lock_guard<std::mutex> lock(submitMutex);
    result_ = vkQueueSubmit(backend.get_queue(), 1, &submitInfo, fence);
    VK_CHECK_RESULT_NOT_EXIT(result_);
}


temp_command_execute::~temp_command_execute() {
    const auto &backend = VK_backend::instance();

    vkEndCommandBuffer(commandBuffer);

    command_submit submit(1, &commandBuffer, fence_);

    std::lock_guard<std::mutex> lock(submit.get_mutex());
    vkQueueWaitIdle(backend.get_queue());
    // 应该是这里导致了速度慢了很多.
    if (commandBuffer != VK_NULL_HANDLE)
        vkFreeCommandBuffers(backend.get_device(), pool, 1, &commandBuffer);
    if (pool != VK_NULL_HANDLE)
        vkDestroyCommandPool(backend.get_device(), pool, nullptr);
}


void temp_command_execute::add_execute_function(
    const std::function<void(VkCommandBuffer commandBuffer)> &callback, VkFence fence) {
    callback(commandBuffer);
    fence_ = fence;
}

temp_command_execute::temp_command_execute() {
    const auto &backend = VK_backend::instance();
    // pool // 是需要申请的

    const VkCommandPoolCreateInfo commandPoolCI{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = backend.get_queue_Family()
    };
    VK_CHECK_RESULT(vkCreateCommandPool(backend.get_device(), &commandPoolCI, nullptr, &pool));
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool        = pool;
    allocInfo.commandBufferCount = 1;
    //  todo : vkAllocateCommandBuffers 必须加锁
    vkAllocateCommandBuffers(backend.get_device(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
}
