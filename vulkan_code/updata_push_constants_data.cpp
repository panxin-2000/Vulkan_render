//
// Created by 潘鑫 on 2026/2/15.
//

#include "update_push_constants_data.h"
#include "vulkan_device_handle.h"

uniform_buffer *buffer = nullptr;

uniform_buffer &get_uniform_buffer() {
    auto &handle = VK_handle::get();

    if (buffer == nullptr) {
        buffer = new uniform_buffer();
        VkBufferCreateInfo uBufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size  = 32 * 1024, // 32K
            .usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
        };
        VmaAllocationCreateInfo uBufferAllocCI{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                     VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
                     VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };
        VK_CHECK_RESULT_NOT_EXIT(
                                 vmaCreateBuffer(handle.get_allocator(),
                                     &uBufferCI,
                                     &uBufferAllocCI,
                                     &buffer->buffer,
                                     &buffer->allocation,
                                     nullptr));
        buffer->memory_pool.push_back({0, 32 * 1024, false});
    }
    return *buffer;
}
