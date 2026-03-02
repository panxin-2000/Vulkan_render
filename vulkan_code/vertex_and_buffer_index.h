//
// Created by 潘鑫 on 2026/1/26.
//

#ifndef HOWTOVULKAN_VERTEX_AND_BUFFER_INDEX_H
#define HOWTOVULKAN_VERTEX_AND_BUFFER_INDEX_H
#include <volk.h>
#include "vulkan_device_handle.h"

#include "vulkan_buffer.h"


// 最差结果 总是 CPU 可见, GPU 通过 PCIE 读取数据
inline VKR_buffer_ptr create_staging_buffer(const VK_handle &handle, VkDeviceSize size) {
    return create_vma_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
}

inline VKR_buffer_ptr create_vertex_index_buffer(const VK_handle &handle, const VkDeviceSize size) {
    return create_vma_buffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                             VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                             VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT); // 最差结果 纯显存（DEVICE_LOCAL）
}

/**
 *  总会创建成功，除非内存不够，返回 都为 VK_NULL_HANDLE
 * @param handle
 * @param size
 * @param mem_copy_callback
 * @return
 */
inline VKR_buffer_ptr create_vertex_index_buffer(const VK_handle &handle, VkDeviceSize size,
                                                 std::function<void(void *)> mem_copy_callback) {
    auto vBuffer = create_vertex_index_buffer(handle, size);
    if (vBuffer->empty())
        return {};
    // 没有创建成功，直接退出
    // 创建成功，之后，记录，还是？

    if (vBuffer->host_visible() == false) {
        LOG_INFO(g_log(), "can find a cpu write memory, only get GPU memory", size);
        auto staging_buffer = create_staging_buffer(handle, size);
        if (staging_buffer->empty()) {
            // 创建 staging_buffer 失败
            vBuffer->DestroyBuffer();
            return {};
        }

        if (staging_buffer->host_visible() == false) {
            LOG_INFO(g_log(), "can find a cpu write memory, allocate size {}", size);
        } else {
            copy_mem_from_cpu_to_gpu(staging_buffer, mem_copy_callback);
            copy_vk_buffer_and_execution(staging_buffer, vBuffer, size);
        }
        staging_buffer->DestroyBuffer();
    } else {
        // 创建成功，但是 map 不成功的很少见
        copy_mem_from_cpu_to_gpu(vBuffer, mem_copy_callback);
    }
    return vBuffer;
}


#endif //HOWTOVULKAN_VERTEX_AND_BUFFER_INDEX_H
