//
// Created by 潘鑫 on 2026/1/23.
//

#ifndef HELLO_MAC_VULKAN_BUFFER_H
#define HELLO_MAC_VULKAN_BUFFER_H
#include "vulkan_device_handle.h"
#include "vulkan_global_macro.h"


void copy_vk_buffer_and_execution(const VKDevice &handle, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

void end_and_submit_one_command_buffer(const VKDevice &handle, VkCommandBuffer commandBuffer);

VkCommandBuffer begin_one_command_buffer(const VKDevice &handle);


#endif //HELLO_MAC_VULKAN_BUFFER_H
