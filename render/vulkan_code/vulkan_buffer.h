//
// Created by 潘鑫 on 2026/1/23.
//

#ifndef HELLO_MAC_VULKAN_BUFFER_H
#define HELLO_MAC_VULKAN_BUFFER_H

#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <set>
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
class VKR_buffer_block;
using VKR_buffer_block_ptr = std::shared_ptr<VKR_buffer_block>;


class VKR_buffer_block {
public:
    VKR_buffer_block(const VkBuffer buffer,
                     const VkDeviceSize offset,
                     const VkDeviceSize size,
                     const VmaAllocation allocation_handle         = VK_NULL_HANDLE,
                     const VmaVirtualAllocation virtual_allocation = VK_NULL_HANDLE) {
        buffer_handle_      = buffer;
        allocation_         = allocation_handle;
        virtual_allocation_ = virtual_allocation;
        offset_             = offset;
        size_               = size;
    }


    [[nodiscard]] VkDeviceSize get_offset() const {
        return offset_;
    }

    [[nodiscard]] VkDeviceSize get_size() const {
        return size_;
    }

    [[nodiscard]] VkDeviceAddress get_gpu_device_address() const;

    [[nodiscard]] VkBuffer get_buffer_handle(const uint64_t timeline = 0) const {
        return buffer_handle_;
    }

    [[nodiscard]] VmaAllocation get_allocation_handle(const uint64_t timeline = 0) const {
        return allocation_;
    }

    [[nodiscard]] VmaVirtualAllocation get_virtual_allocation_(const uint64_t timeline = 0) const {
        return virtual_allocation_;
    }

    [[nodiscard]] const VkBuffer *get_buffer_handle_ptr(const uint64_t timeline = 0) const {
        return &buffer_handle_;
    }

    bool empty() const {
        if (buffer_handle_ == VK_NULL_HANDLE ||
            virtual_allocation_ == VK_NULL_HANDLE ||
            allocation_ == VK_NULL_HANDLE) {
            return true;
        } else {
            return false;
        }
    }


    VKR_buffer_block() = default;


    VkBuffer buffer_handle_                  = VK_NULL_HANDLE;
    VmaAllocation allocation_                = VK_NULL_HANDLE;
    VmaVirtualAllocation virtual_allocation_ = VK_NULL_HANDLE;
    VkDeviceSize offset_                     = 0;
    VkDeviceSize size_                       = 0;
};


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


    [[nodiscard]] const VKR_buffer &value() const {
        return *this;
    }

    bool copy_mem_from_cpu_to_gpu_host_visible(const std::function<void(void *)> &mem_copy_callback);

    bool copy_mem_from_cpu_to_gpu(const std::function<void(void *)> &mem_copy_callback,
                                  VkDeviceSize size);


    [[nodiscard]] void *mapped_address() const;

    [[nodiscard]] bool unmap_memory() const;

    [[nodiscard]] void *map_memory() const;

    [[nodiscard]] const VkBuffer get_buffer_handle(const uint64_t timeline = 0);

    [[nodiscard]] const VmaAllocation get_allocation_handle(const uint64_t timeline = 0);

    [[nodiscard]] const VkBuffer *get_buffer_handle_ptr(const uint64_t timeline = 0);

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


void copy_vk_buffer_and_execution(const VKR_buffer_ptr &srcBuffer,
                                  const VKR_buffer_ptr &dstBuffer,
                                  VkDeviceSize size,
                                  VkDeviceSize srcOffset = 0,
                                  VkDeviceSize dstOffset = 0);


bool copy_mem_from_cpu_to_gpu(const VKR_buffer_ptr &buffer, const std::function<void(void *)> &mem_copy_callback);

VKR_buffer_ptr create_vma_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaAllocationCreateFlags flags);

void discard_buffer_map_clean(uint64_t finished_timeline);


class VKR_buffer_pool {
public:
    VKR_buffer_pool(const VkDeviceSize size,
                    const VkBufferUsageFlags usage,
                    const VmaAllocationCreateFlags flags) {
        buffer_                                   = create_vma_buffer(size, usage, flags);
        VmaVirtualBlockCreateInfo blockCreateInfo = {};
        blockCreateInfo.size                      = size;
        const VkResult res                        = vmaCreateVirtualBlock(&blockCreateInfo, &block_);
        assert(res == VK_SUCCESS);
    }

    VKR_buffer_pool() = default;

    VKR_buffer_block alloc(const VkDeviceSize size, const VkDeviceSize alignment) {
        assert(size > 0);
        VmaVirtualAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.size                           = size;
        allocCreateInfo.alignment                      = alignment;
        VmaVirtualAllocation allocation_handle;
        VkDeviceSize offset;
        const auto res = vmaVirtualAllocate(block_, &allocCreateInfo, &allocation_handle, &offset);
        if (res == VK_SUCCESS) {
            allocations_.emplace(allocation_handle);
            return {
                buffer_->get_buffer_handle(),
                get_offset(allocation_handle),
                get_size(allocation_handle),
                buffer_->get_allocation_handle(),
                allocation_handle
            };
        } else {
            return {VK_NULL_HANDLE, 0, 0,VK_NULL_HANDLE,VK_NULL_HANDLE};
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

    void free(const VmaVirtualAllocation allocation_handle) {
        allocations_.erase(allocation_handle);
        need_free_.emplace(allocation_handle);
    }

    void clean_free_blocks() {
        for (const auto free_block: need_free_) {
            dealloc(free_block);
        }
        need_free_.clear();
    }

    [[nodiscard]] const VmaAllocation get_allocation_handle(const uint64_t timeline = 0) {
        return buffer_->get_allocation_handle(timeline);
    }

    ~VKR_buffer_pool() {
        for (const auto free_block: allocations_) {
            dealloc(free_block);
        }
        allocations_.clear();
        for (const auto free_block: need_free_) {
            dealloc(free_block);
        }
        need_free_.clear();
        if (block_ != VK_NULL_HANDLE)
            vmaDestroyVirtualBlock(block_);
        block_  = VK_NULL_HANDLE;
        buffer_ = nullptr;
    }

private:
    void dealloc(const VmaVirtualAllocation allocation_handle) const {
        vmaVirtualFree(block_, allocation_handle);
    }

    std::set<VmaVirtualAllocation> allocations_;
    std::set<VmaVirtualAllocation> need_free_;
    VmaVirtualBlock block_;
    VKR_buffer_ptr buffer_;
};


#endif //HELLO_MAC_VULKAN_BUFFER_H
