//
// Created by 潘鑫 on 2026/1/23.
//

#ifndef HELLO_MAC_VULKAN_BUFFER_H
#define HELLO_MAC_VULKAN_BUFFER_H

#include <map>
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
    VKR_buffer(const VkBuffer buffer_handle, const VmaAllocation allocation) : buffer_handle_(buffer_handle),
                                                                               allocation_(allocation) {
    }

    ~VKR_buffer();


    [[nodiscard]] void *mapped_address() const;

    [[nodiscard]] VkDeviceAddress get_gpu_device_address() const;

    [[nodiscard]] bool host_visible() const;

    [[nodiscard]] bool need_flush() const;

    // timeline 会和这个函数强关联
    [[nodiscard]] VkBuffer get_buffer_handle(const uint64_t timeline = 0) {
        if (timeline > timeline_) timeline_ = timeline;
        return buffer_handle_;
    }

    // timeline 会和这个函数强关联
    [[nodiscard]] const VkBuffer *get_buffer_handle_ptr(const uint64_t timeline = 0) {
        if (timeline > timeline_) timeline_ = timeline;
        return &buffer_handle_;
    }

    bool unmap_memory() const;

    bool destroy_buffer();

    [[nodiscard]] const VKR_buffer &value() const {
        return *this;
    }

    [[nodiscard]] void *map_memory() const;

    [[nodiscard]] VkDeviceSize complete_size() const;

    [[nodiscard]] static VkDeviceSize get_offset() {
        return 0;
    }

    bool flush(VkDeviceSize offset = 0, VkDeviceSize size = 0) const;

    [[nodiscard]] bool empty() const {
        if (buffer_handle_ == VK_NULL_HANDLE || allocation_ == VK_NULL_HANDLE) {
            return true;
        } else {
            return false;
        }
    }
};

class VKR_buffer_pool : public VKR_buffer {
public:
    VKR_buffer_pool(const VkBuffer buffer_handle,
                    const VmaAllocation allocation) : VKR_buffer(buffer_handle, allocation) {
        offset_and_size_map.insert({0, {complete_size(), true}});
        size_and_offset_map.insert({complete_size(), {0}});
    }

    std::map<VkDeviceSize, size_and_status> &get_offset_and_size_map() {
        return offset_and_size_map;
    };

    std::multimap<VkDeviceSize, offset_no_status> &get_size_and_offset_map() {
        return size_and_offset_map;
    };

private:
    std::map<VkDeviceSize, size_and_status> offset_and_size_map;
    std::multimap<VkDeviceSize, offset_no_status> size_and_offset_map;
};

class VKR_buffer_block {
public:
    VKR_buffer_block(const VKR_buffer_pool_ptr &buffer,
                     const VkDeviceSize offset,
                     const VkDeviceSize size) : ptr(buffer) {
        offset_ = offset;
        size_   = size;
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

    bool destroy_buffer();

    VKR_buffer_block() = default;

    ~VKR_buffer_block();

    VKR_buffer_pool_ptr ptr  = nullptr; // 指向 VKR_buffer 的 指针
    VkDeviceSize offset_     = 0;
    VkDeviceSize size_       = 0;
    uint64_t block_timeline_ = 0;
};


// VKR_buffer_block_ptr 是一个指针 指向了 VKR_buffer_block
//                                      VKR_buffer_block 中有一个 指针 ，指向了  VKR_buffer
//    VKR_buffer 中有   VkBuffer        和      VmaAllocation
//                   VkBuffer是一个句柄        VmaAllocation是一个指针

// VKR_buffer_ptr 另外一个方式，不进行池化，直接 通过指向 指向 VKR_buffer

VKR_buffer_block_ptr GPU_pool_alloc(const VKR_buffer_pool_ptr &buffer, uint64_t request_size);

void copy_vk_buffer_and_execution(const VKR_buffer_ptr &srcBuffer, const VKR_buffer_ptr &dstBuffer, VkDeviceSize size);

void end_and_submit_one_command_buffer(VkCommandBuffer commandBuffer);

VkCommandBuffer begin_one_command_buffer();

bool copy_mem_from_cpu_to_gpu(const VKR_buffer_ptr &buffer, const std::function<void(void *)> &mem_copy_callback);

VKR_buffer_ptr create_vma_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaAllocationCreateFlags flags);

void discard_buffer_map_clean();

using buffer_offset = VkDeviceSize;

std::mutex &get_vkQueueSubmit_mutex();

#endif //HELLO_MAC_VULKAN_BUFFER_H
