//
// Created by 潘鑫 on 2026/2/15.
//

#include "update_push_constants_data.h"
#include "vulkan_backend.h"

VKR_buffer_pool_ptr buffer = nullptr;

VKR_buffer_pool_ptr &get_uniform_buffer() {
    auto &handle = VK_backend::instance();

    if (buffer == nullptr) {
        VkBuffer vBuffer{VK_NULL_HANDLE};
        VmaAllocation vBufferAllocation{VK_NULL_HANDLE};

        VkBufferCreateInfo uBufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size  = 1024 * 1024, // 1M 需要申请1M 之前的时候应该是 128K 就能够存储完成的
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
        buffer = std::make_shared<VKR_buffer_pool>(vBuffer, vBufferAllocation);
    }
    // auto complete_sghize = (*buffer)->complete_size();
    return buffer;
}

VKR_buffer_ptr create_SSBO_buffer(const VkDeviceSize &size) {
    return create_vma_buffer(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                   VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT |
                                   VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT,
                             VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                             VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT);
}

/**
 * 这里是直接复制的函数，如果想做 多线程 上传的话，那么其实必须顶一个两个函数，
 * 一个需要注意的地方是 原本资源想要放置在哪里必须确定好，最后放置在哪里也是需要确定好的
 * 一个是负责具体复制的函数，
 * 另一个是复制完成之后资源是否需要释放的函数
 * @param entity
 * @param src
 * @param size
 */
VKR_buffer_ptr copy_data_to_gpu_memory(const void *src, uint64_t size) {
    auto temp_ptr          = create_SSBO_buffer(ALIGN_1024(size));
    auto mem_copy_function = [src,size](void *dst) {
        memcpy(dst, src, size);
    };
    copy_mem_from_cpu_to_gpu(temp_ptr, mem_copy_function);

    return temp_ptr;
}
