//
// Created by 潘鑫 on 2026/7/22.
//

#include "vulkan_execute_command.h"
#include "../engine.h"
#include "vulkan_backend.h"


temp_command_execute::~temp_command_execute() {
    const auto &backend = VK_backend::instance();

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer; {
        std::lock_guard<std::mutex> lock(get_vkQueueSubmit_mutex());
        vkQueueSubmit(backend.get_queue(), 1, &submitInfo, VK_NULL_HANDLE);
    }
    vkQueueWaitIdle(backend.get_queue());
    vkFreeCommandBuffers(backend.get_device(), Engine::instance().get_command_pool(), 1, &commandBuffer);
}


void temp_command_execute::add_execute_function(
    const std::function<void(VkCommandBuffer commandBuffer)> &callback) const {
    callback(commandBuffer);
}

temp_command_execute::temp_command_execute() {
    const auto &backend = VK_backend::instance();
    // pool // 是需要申请的
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool        = Engine::instance().get_command_pool(); // 那么确实是，这里不应该包含一个engine池的
    allocInfo.commandBufferCount = 1;

    //  todo : vkAllocateCommandBuffers 必须加锁
    vkAllocateCommandBuffers(backend.get_device(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
}
