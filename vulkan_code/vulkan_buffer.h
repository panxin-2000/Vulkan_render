//
// Created by 潘鑫 on 2026/1/23.
//

#ifndef HELLO_MAC_VULKAN_BUFFER_H
#define HELLO_MAC_VULKAN_BUFFER_H
#include "vulkan_device_handle.h"
#include "vulkan_global_macro.h"

void createBuffer(VKDevice &handle, VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags properties, VkBuffer &buffer,
                  VkDeviceMemory &bufferMemory);

#endif //HELLO_MAC_VULKAN_BUFFER_H
