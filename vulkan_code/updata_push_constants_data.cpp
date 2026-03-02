//
// Created by 潘鑫 on 2026/2/15.
//

#include "update_push_constants_data.h"
#include "vulkan_device_handle.h"

VKR_buffer_ptr *buffer = nullptr;

VKR_buffer_ptr &get_uniform_buffer() {
    auto &handle = VK_handle::get();

    if (buffer == nullptr) {
        VkBuffer vBuffer{VK_NULL_HANDLE};
        VmaAllocation vBufferAllocation{VK_NULL_HANDLE};

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
                                     &vBuffer,
                                     &vBufferAllocation,
                                     nullptr));
        buffer = new VKR_buffer_ptr(vBuffer, vBufferAllocation);
    }
    return *buffer;
}
