//
// Created by 潘鑫 on 2026/1/23.
//

#include "vulkan_buffer.h"

#include "vulkan_device_handle.h"
#include "vulkan_image.h"


void copy_vk_buffer(const VKDevice &handle, VkBuffer srcBuffer,
                VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(handle);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size      = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(handle, commandBuffer);
}
