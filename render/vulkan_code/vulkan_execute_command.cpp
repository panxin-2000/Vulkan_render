//
// Created by 潘鑫 on 2026/7/22.
//

#include "vulkan_execute_command.h"
#include "vulkan_backend.h"
#include "vulkan_buffer.h"

std::mutex Command_submit_manager::submitMutex_;
std::vector<std::function<void(VkCommandBuffer commandBuffer, uint64_t time_line)> >
Command_submit_manager::callback_functions_;

VkResult Command_submit_manager::command_copy_image_to_screen(const VkPresentInfoKHR &presentInfo) {
    const auto &backend = VK_backend::instance();
    std::lock_guard<std::mutex> lock(submitMutex_);
    return vkQueuePresentKHR(backend.get_queue(), &presentInfo);
}

bool Command_submit_manager::command_buffer_submit(const uint32_t commandBufferCount,
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
    std::lock_guard<std::mutex> lock(submitMutex_);
    VkResult result_ = vkQueueSubmit(backend.get_queue(), 1, &submitInfo, fence);
    VK_CHECK_RESULT_NOT_EXIT(result_);
    return true;
}


void Command_submit_manager::add_execute_function(
    const std::function<void(VkCommandBuffer commandBuffer, uint64_t time_line)> &callback, VkFence fence) {
    callback_functions_.push_back(callback);
    // fence_ = fence;
}


void Command_submit_manager::destroy() const {
    const auto &backend = VK_backend::instance();
    if (commandBuffer != VK_NULL_HANDLE)
        vkFreeCommandBuffers(backend.get_device(), pool, 1, &commandBuffer);
    if (pool != VK_NULL_HANDLE)
        vkDestroyCommandPool(backend.get_device(), pool, nullptr);
}

void Command_submit_manager::create() {
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
}

void Command_submit_manager::execute_callback_functions(const uint64_t time_line) {
    if (callback_functions_.empty()) return;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    for (auto callback: callback_functions_) {
        callback(commandBuffer, time_line);
        // 数量多起来的时候也是很慢的 // 1000多的时候就很慢了
    }
    callback_functions_.clear();


    vkEndCommandBuffer(commandBuffer);

    command_buffer_submit(1, &commandBuffer);

    // 这里那么其实存在另一个问题,那就是需要
    // 添加一个标志,用于询问是否已经上传完成
    // 已经上传完成了
}
