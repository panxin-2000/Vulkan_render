//
// Created by 潘鑫 on 2026/1/23.
//

#include "vulkan_buffer.h"

#include "vulkan_device_handle.h"
#include "vulkan_image.h"

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


void copyBuffer(VKDevice &handle, VkBuffer srcBuffer,
                VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(handle);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size      = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(handle, commandBuffer);
}

void createVertexBuffer(VKDevice &handle,
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
    copyBuffer(handle, stagingBuffer, verticesBuffer, bufferSize);
    vkDestroyBuffer(handle.get_device(), stagingBuffer, nullptr);
    vkFreeMemory(handle.get_device(), stagingBufferMemory, nullptr);
}

// 这个函数和上一个函数是一样的
void createIndexBuffer(VKDevice &handle,
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
    copyBuffer(handle, stagingBuffer, verticesBuffer, bufferSize);
    vkDestroyBuffer(handle.get_device(), stagingBuffer, nullptr);
    vkFreeMemory(handle.get_device(), stagingBufferMemory, nullptr);
}
