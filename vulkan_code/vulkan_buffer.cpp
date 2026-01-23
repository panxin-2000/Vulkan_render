//
// Created by 潘鑫 on 2026/1/23.
//

#include "vulkan_buffer.h"

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

void createBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags properties, VkBuffer &buffer,
                  VkDeviceMemory &bufferMemory) {
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertices buffer !");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    //        VkMemoryAllocateInfo memAllocateInfo;之前错误比较严重的地方是在这里？
    //        在Linux上会出这个错吗？应该不会吧在glm的时候也碰到过为初始化好导致的错误。
    VkMemoryAllocateInfo memAllocateInfo{};
    memAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocateInfo.allocationSize = memRequirements.size;
    memAllocateInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits,
                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    //      下面一行有个错误，以改正
    if (vkAllocateMemory(device, &memAllocateInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory !");
    }
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}
