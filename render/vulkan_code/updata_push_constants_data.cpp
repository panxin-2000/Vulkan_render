//
// Created by 潘鑫 on 2026/2/15.
//

#include "vulkan_global_macro.h"
#include "update_push_constants_data.h"
#include "vertex_and_buffer_index.h"
#include "vulkan_backend.h"



VKR_buffer_ptr create_SSBO_buffer(const VkDeviceSize size) {
    return create_vma_buffer(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                   VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT_KHR |
                                   VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT_KHR,
                             VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                             VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT);
}

/**
 * 这里是直接复制的函数，如果想做 多线程 上传的话，那么其实必须顶一个两个函数，
 * 一个需要注意的地方是 原本资源想要放置在哪里必须确定好，最后放置在哪里也是需要确定好的
 * 一个是负责具体复制的函数，
 * 另一个是复制完成之后资源是否需要释放的函数
 * @param size
 * @param src
 */
VKR_buffer_ptr create_SSBO_buffer(const VkDeviceSize size, const void *src) {
    if (size > 0) {
        const auto &backend = VK_backend::instance();
        auto temp_ptr       = create_SSBO_buffer(ALIGN_1024(size));
        if (src != nullptr) {
            auto mem_copy_function = [src,size](void *dst) {
                memcpy(dst, src, size);
            };
            return create_base_buffer(temp_ptr, backend, size, mem_copy_function);
        }
        return temp_ptr;
    }
    return nullptr;
}
