//
// Created by 潘鑫 on 2026/1/23.
//

#ifndef HELLO_MAC_VULKAN_BUFFER_H
#define HELLO_MAC_VULKAN_BUFFER_H
#include "vulkan_device_handle.h"
#include "vulkan_global_macro.h"


void copy_vk_buffer_and_execution(const VK_handle &handle, VKR_buffer_ptr srcBuffer, VKR_buffer_ptr dstBuffer,
                                  VkDeviceSize size);

void end_and_submit_one_command_buffer(const VK_handle &handle, VkCommandBuffer commandBuffer);

VkCommandBuffer begin_one_command_buffer(const VK_handle &handle);

bool copy_mem_from_cpu_to_gpu(const VKR_buffer_ptr &buffer,
                              const std::function<void(void *)> &mem_copy_callback);

VKR_buffer_ptr create_vma_buffer(VkDeviceSize size,
                                 VkBufferUsageFlags usage, VmaAllocationCreateFlags flags);
#endif //HELLO_MAC_VULKAN_BUFFER_H
