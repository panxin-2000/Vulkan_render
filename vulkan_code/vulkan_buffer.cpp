//
// Created by 潘鑫 on 2026/1/23.
//

#include "vulkan_buffer.h"

#include "vulkan_device_handle.h"
#include "vulkan_image.h"


void copy_vk_buffer_and_execution(const VK_handle &handle, VkBuffer srcBuffer,
                                  VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = begin_one_command_buffer(handle);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size      = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    end_and_submit_one_command_buffer(handle, commandBuffer);
}


void end_and_submit_one_command_buffer(const VK_handle &handle, VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer;

    vkQueueSubmit(handle.get_queue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(handle.get_queue());

    vkFreeCommandBuffers(handle.get_device(), handle.get_command_pool(), 1, &commandBuffer);
}


VkCommandBuffer begin_one_command_buffer(const VK_handle &handle) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool        = handle.get_command_pool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    //  todo : vkAllocateCommandBuffers 必须加锁
    vkAllocateCommandBuffers(handle.get_device(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}
