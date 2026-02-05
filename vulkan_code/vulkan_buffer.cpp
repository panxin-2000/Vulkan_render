//
// Created by 潘鑫 on 2026/1/23.
//

#include "vulkan_buffer.h"

#include "vulkan_device_handle.h"

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

void createBuffer(VKDevice &handle, VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags properties, VkBuffer &buffer,
                  VkDeviceMemory &bufferMemory) {
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size        = size;
    bufferCreateInfo.usage       = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(handle.get_device(), &bufferCreateInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertices buffer !");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(handle.get_device(), buffer, &memRequirements);

    //        VkMemoryAllocateInfo memAllocateInfo;之前错误比较严重的地方是在这里？
    //        在Linux上会出这个错吗？应该不会吧在glm的时候也碰到过为初始化好导致的错误。
    VkMemoryAllocateInfo memAllocateInfo{};
    memAllocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocateInfo.allocationSize  = memRequirements.size;
    memAllocateInfo.memoryTypeIndex = findMemoryType(handle.physical_device_, memRequirements.memoryTypeBits,
                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    //      下面一行有个错误，以改正
    if (vkAllocateMemory(handle.get_device(), &memAllocateInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory !");
    }
    vkBindBufferMemory(handle.get_device(), buffer, bufferMemory, 0);
}


VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool        = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer,
                           VkQueue graphicsQueue) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}


void copyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer srcBuffer,
                VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size      = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(device, commandPool, commandBuffer, graphicsQueue);
}

void createVertexBuffer(VKDevice &handle,
                        VkCommandPool commandPool, VkQueue graphicsQueue,
                        void *buffer_data, uint32_t size, VkBuffer &verticesBuffer,
                        VkDeviceMemory &vertexBufferMemory) {
    // VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();
    VkDeviceSize bufferSize = size;

    VkBuffer stagingBuffer{};
    VkDeviceMemory stagingBufferMemory{};
    createBuffer(handle, bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(handle.get_device(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, buffer_data, (size_t) bufferSize);
    //还有其他的传递数值的办法吗？知道memcpy可能会被内核给优化过，最终调用DMA
    //有明显提示调用DMA的函数吗？在这里如何创建多个线程来完成这个工作？
    //实际中有可能减少这一步吗？
    vkUnmapMemory(handle.get_device(), stagingBufferMemory); //参数错误，以改正
    createBuffer(handle, bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, verticesBuffer, vertexBufferMemory);
    copyBuffer(handle.get_device(), commandPool, graphicsQueue, stagingBuffer, verticesBuffer, bufferSize);
    vkDestroyBuffer(handle.get_device(), stagingBuffer, nullptr);
    vkFreeMemory(handle.get_device(), stagingBufferMemory, nullptr);
}

// 这个函数和上一个函数是一样的
void createIndexBuffer(VKDevice &handle,
                       VkCommandPool commandPool, VkQueue graphicsQueue,
                       void *buffer_data, uint32_t size, VkBuffer &verticesBuffer,
                       VkDeviceMemory &vertexBufferMemory) {
    VkDeviceSize bufferSize = size;
    VkBuffer stagingBuffer{};
    VkDeviceMemory stagingBufferMemory{};
    createBuffer(handle, bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);
    void *data;
    vkMapMemory(handle.get_device(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, buffer_data, (size_t) bufferSize);
    vkUnmapMemory(handle.get_device(), stagingBufferMemory);
    createBuffer(handle, bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, verticesBuffer, vertexBufferMemory);
    copyBuffer(handle.get_device(), commandPool, graphicsQueue, stagingBuffer, verticesBuffer, bufferSize);
    vkDestroyBuffer(handle.get_device(), stagingBuffer, nullptr);
    vkFreeMemory(handle.get_device(), stagingBufferMemory, nullptr);
}
