//
// Created by 潘鑫 on 2026/1/23.
//

#ifndef HELLO_MAC_VULKAN_BUFFER_H
#define HELLO_MAC_VULKAN_BUFFER_H

#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <utility>
#include <vk_mem_alloc.h>
#include "APP_utility_mixins.h"

struct size_and_status {
    uint64_t size_ = 0;
    bool status_   = false;
};

struct offset_no_status {
    uint64_t offset_ = 0;
};

class VKR_buffer;
using VKR_buffer_ptr = std::shared_ptr<VKR_buffer>;
class VKR_buffer_pool;
using VKR_buffer_pool_ptr = std::shared_ptr<VKR_buffer_pool>;
class VKR_buffer_block;
using VKR_buffer_block_ptr = std::shared_ptr<VKR_buffer_block>;

class VKR_buffer : public NonCopyable {
protected:
    VkBuffer buffer_handle_   = VK_NULL_HANDLE;
    VmaAllocation allocation_ = VK_NULL_HANDLE;
    uint64_t timeline_        = 0;

public:
    VKR_buffer(const VkBuffer buffer_handle, const VmaAllocation allocation);

    ~VKR_buffer();

    bool destroy_buffer();

    [[nodiscard]] VkDeviceAddress get_gpu_device_address() const;

    [[nodiscard]] bool host_visible() const;

    [[nodiscard]] bool need_flush() const;

    [[nodiscard]] const VkBuffer get_buffer_handle(const uint64_t timeline = 0);

    [[nodiscard]] const VkBuffer *get_buffer_handle_ptr(const uint64_t timeline = 0);

    [[nodiscard]] const VKR_buffer &value() const {
        return *this;
    }

    bool copy_mem_from_cpu_to_gpu_host_visible(const std::function<void(void *)> &mem_copy_callback);

    bool copy_mem_from_cpu_to_gpu(const std::function<void(void *)> &mem_copy_callback,
                                  VkDeviceSize size);


    [[nodiscard]] void *mapped_address() const;

    [[nodiscard]] bool unmap_memory() const;

    [[nodiscard]] void *map_memory() const;

    [[nodiscard]] VkDeviceSize get_size() const;

    [[nodiscard]] static VkDeviceSize get_offset() {
        return 0;
    }

    bool flush(VkDeviceSize offset = 0, VkDeviceSize size = 0) const;

    [[nodiscard]] bool empty() const;
};

[[nodiscard]] inline bool check_need_reserve(const VKR_buffer_ptr &buffer, const VkDeviceSize size) {
    if (buffer != nullptr && buffer->get_size() > size) {
        return true;
    }
    return false;
}

[[nodiscard]] bool append_data(VKR_buffer_ptr &buffer,
                               const VkDeviceSize offset,
                               const std::function<void(void *)> &mem_copy_callback,
                               const VkDeviceSize append_size);

class VKR_buffer_offset {
public:
    VKR_buffer_offset() {
    };

    VKR_buffer_offset(const VKR_buffer_ptr &buffer_ptr,
                      const VkDeviceSize size,
                      const VkDeviceSize offset = 0) : buffer_ptr_(buffer_ptr),
                                                       offset_(offset), size_(size) {
    }

    VKR_buffer_offset(const VKR_buffer_ptr &buffer_ptr) : buffer_ptr_(buffer_ptr) {
        offset_ = 0;
        size_   = buffer_ptr->get_size();
    }

    [[nodiscard]] VkDeviceAddress get_gpu_device_address() const {
        return buffer_ptr_->get_gpu_device_address() + offset_;
    }

    [[nodiscard]] VkBuffer get_buffer_handle(const uint64_t timeline = 0) const {
        return buffer_ptr_->get_buffer_handle(timeline);
    }

    [[nodiscard]] VkDeviceSize get_offset() const {
        return offset_;
    }

    [[nodiscard]] VkDeviceSize get_size() const {
        return size_;
    }

    void clear() {
        buffer_ptr_ = nullptr;
        offset_     = 0;
        size_       = 0;
    }

private:
    VKR_buffer_ptr buffer_ptr_ = nullptr;
    VkDeviceSize offset_       = 0;
    VkDeviceSize size_         = 0;
};

class VKR_buffer_pool : public VKR_buffer {
public:
    VKR_buffer_pool(const VkBuffer buffer_handle,
                    const VmaAllocation allocation) : VKR_buffer(buffer_handle, allocation) {
        VmaVirtualBlockCreateInfo blockCreateInfo = {};
        blockCreateInfo.size                      = VKR_buffer::get_size();
        const VkResult res                        = vmaCreateVirtualBlock(&blockCreateInfo, &block_);
        assert(res == VK_SUCCESS);
    }

    VmaVirtualAllocation alloc(const VkDeviceSize size, const VkDeviceSize alignment) const {
        assert(size > 0);
        VmaVirtualAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.size                           = size;
        allocCreateInfo.alignment                      = alignment;
        VmaVirtualAllocation allocation_handle;
        VkDeviceSize offset;
        const auto res = vmaVirtualAllocate(block_, &allocCreateInfo, &allocation_handle, &offset);
        if (res == VK_SUCCESS) {
            return allocation_handle;
        } else {
            return VK_NULL_HANDLE;
        }
    }

    VkDeviceSize get_offset(const VmaVirtualAllocation allocation_handle) const {
        VmaVirtualAllocationInfo allocInfo;
        vmaGetVirtualAllocationInfo(block_, allocation_handle, &allocInfo);
        return allocInfo.offset;
    }

    VkDeviceSize get_size(const VmaVirtualAllocation allocation_handle) const {
        VmaVirtualAllocationInfo allocInfo;
        vmaGetVirtualAllocationInfo(block_, allocation_handle, &allocInfo);
        return allocInfo.size;
    }

    void dealloc(const VmaVirtualAllocation allocation_handle) const {
        vmaVirtualFree(block_, allocation_handle);
    }

    ~VKR_buffer_pool() {
        vmaDestroyVirtualBlock(block_);
    }

private:
    VmaVirtualBlock block_;
};


inline void GPU_pool_free(const VKR_buffer_pool_ptr &buffer, const VmaVirtualAllocation allocation_handle) {
    buffer->dealloc(allocation_handle);
}


class VKR_buffer_block {
public:
    VKR_buffer_block(const VKR_buffer_pool_ptr &buffer,
                     const VmaVirtualAllocation allocation_handle,
                     const VkDeviceSize offset,
                     const VkDeviceSize size) : ptr(buffer) {
        allocation_handle_ = allocation_handle;
        offset_            = offset;
        size_              = size;
    }

    [[nodiscard]] VkBuffer get_buffer_handle(const uint64_t timeline = 0) {
        if (timeline > block_timeline_) block_timeline_ = timeline;
        return ptr->get_buffer_handle(timeline);
    }

    [[nodiscard]] const VkBuffer *get_buffer_handle_ptr(const uint64_t timeline = 0) {
        if (timeline > block_timeline_) block_timeline_ = timeline;
        return ptr->get_buffer_handle_ptr(timeline);
    }


    [[nodiscard]] VkDeviceAddress get_gpu_device_address(const uint64_t timeline = 0) {
        if (timeline > block_timeline_) block_timeline_ = timeline;
        return ptr->get_gpu_device_address() + offset_;
    }

    [[nodiscard]] const VKR_buffer_block &value() const {
        return *this;
    }

    VKR_buffer_block() = default;

    ~VKR_buffer_block() {
        GPU_pool_free(ptr, allocation_handle_);
    }

    VKR_buffer_pool_ptr ptr                 = nullptr; // 指向 VKR_buffer 的 指针
    VmaVirtualAllocation allocation_handle_ = VK_NULL_HANDLE;
    VkDeviceSize offset_                    = 0;
    VkDeviceSize size_                      = 0;
    uint64_t block_timeline_                = 0;
};


inline VKR_buffer_block_ptr GPU_pool_alloc(const VKR_buffer_pool_ptr &buffer, const uint64_t request_size) {
    const auto allocation_handle = buffer->alloc(request_size, 16);
    auto block_ptr               = std::make_shared<VKR_buffer_block>(buffer,
                                                        allocation_handle,
                                                        buffer->get_offset(allocation_handle),
                                                        buffer->get_size(allocation_handle));
    return block_ptr;
}


void copy_vk_buffer_and_execution(const VKR_buffer_ptr &srcBuffer, const VKR_buffer_ptr &dstBuffer, VkDeviceSize size,
                                  VkDeviceSize srcOffset = 0,
                                  VkDeviceSize dstOffset = 0);


bool copy_mem_from_cpu_to_gpu(const VKR_buffer_ptr &buffer, const std::function<void(void *)> &mem_copy_callback);

VKR_buffer_ptr create_vma_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaAllocationCreateFlags flags);

void discard_buffer_map_clean(uint64_t finished_timeline);

using buffer_offset = VkDeviceSize;


#endif //HELLO_MAC_VULKAN_BUFFER_H
