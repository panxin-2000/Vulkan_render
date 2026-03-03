//
// Created by 潘鑫 on 2026/1/23.
//

#ifndef HELLO_MAC_VULKAN_BUFFER_H
#define HELLO_MAC_VULKAN_BUFFER_H
#include <list>
#include <map>

#include "vulkan_global_macro.h"
#include <vk_mem_alloc.h>


class VKR_buffer {
protected:
    VkBuffer buffer_handle_   = VK_NULL_HANDLE;
    VmaAllocation allocation_ = VK_NULL_HANDLE;
    uint64_t timeline_        = 0;

    std::map<VkDeviceSize, std::pair<VkDeviceSize, bool> > offset_and_size_map;
    std::map<VkDeviceSize, std::pair<VkDeviceSize, bool> > size_and_offset_map;

public:
    VKR_buffer(const VkBuffer buffer_handle, const VmaAllocation allocation) : buffer_handle_(buffer_handle),
                                                                               allocation_(allocation) {
        offset_and_size_map.insert({0, {complete_size(), true}});
        size_and_offset_map.insert({complete_size(), {0, true}});
    }

    ~VKR_buffer();

    std::map<VkDeviceSize, std::pair<VkDeviceSize, bool> > &get_offset_and_size_map() {
        return offset_and_size_map;
    };

    std::map<VkDeviceSize, std::pair<VkDeviceSize, bool> > &get_size_and_offset_map() {
        return size_and_offset_map;
    };


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

    void *map_memory() const;

    VkDeviceSize complete_size() const;

    bool flush(VkDeviceSize offset = 0, VkDeviceSize size = 0) const;

    [[nodiscard]] bool empty() const {
        if (buffer_handle_ == VK_NULL_HANDLE || allocation_ == VK_NULL_HANDLE) {
            return true;
        } else {
            return false;
        }
    }
};


// using VKR_buffer_ptr = std::shared_ptr<VKR_buffer>;

// 其实只是 std::shared_ptr<VKR_buffer> ptr 的一个封装
class VKR_buffer_ptr {
public:
    VKR_buffer_ptr(const VkBuffer buffer_handle,
                   const VmaAllocation allocation) : ptr(std::make_shared<VKR_buffer>(buffer_handle, allocation)) {
    }

    VKR_buffer_ptr() = default;

    ~VKR_buffer_ptr() = default;

    long use_count() {
        return ptr.use_count();
    }

    void clear() {
        ptr = nullptr;
    }

    explicit operator bool() const noexcept {
        return ptr != nullptr;
    }

    VKR_buffer *operator->() const { return ptr.get(); }
    VKR_buffer *get() const { return ptr.get(); }

protected:
    std::shared_ptr<VKR_buffer> ptr = nullptr;
};


struct address_and_length {
    uint64_t address = 0;
    uint64_t length  = 0;
    bool if_used     = false;
};


class VKR_buffer_block : public VKR_buffer_ptr {
public:
    VKR_buffer_block(const VKR_buffer_ptr &buffer,
                     const VkDeviceSize offset,
                     const VkDeviceSize size) : VKR_buffer_ptr(buffer) {
        offset_ = offset;
        size_   = size;
    }

    [[nodiscard]] VkBuffer get_buffer_handle(const uint64_t timeline = 0) {
        if (timeline > block_timeline_) block_timeline_ = timeline;
        return VKR_buffer_ptr::get()->get_buffer_handle(timeline);
    }

    [[nodiscard]] const VkBuffer *get_buffer_handle_ptr(const uint64_t timeline = 0) {
        if (timeline > block_timeline_) block_timeline_ = timeline;
        return VKR_buffer_ptr::get()->get_buffer_handle_ptr(timeline);
    }


    [[nodiscard]] const VKR_buffer_block &value() const {
        return *this;
    }

    bool destroy_buffer();

    VKR_buffer_block() = default;

    ~VKR_buffer_block();

    VkDeviceSize offset_     = 0;
    VkDeviceSize size_       = 0;
    uint64_t block_timeline_ = 0;
};

VKR_buffer_block GPU_pool_alloc(const VKR_buffer_ptr &buffer, uint64_t size);


class VKR_buffer_block_ptr {
public:
    VKR_buffer_block_ptr(const VKR_buffer_ptr &buffer_handle,
                         const VkDeviceSize size) {
        ptr = std::make_shared<VKR_buffer_block>(GPU_pool_alloc(buffer_handle, size));
    }

    VKR_buffer_block_ptr() = default;

    ~VKR_buffer_block_ptr() = default;

    long use_count() {
        return ptr.use_count();
    }

    void clear() {
        ptr = nullptr;
    }

    explicit operator bool() const noexcept {
        return ptr != nullptr;
    }

    VKR_buffer_block *operator->() const { return ptr.get(); }

protected:
    std::shared_ptr<VKR_buffer_block> ptr = nullptr;
};


class VKR_buffer_pool : public VKR_buffer_ptr {
public:
    VKR_buffer_pool(const VkBuffer buffer_handle, const VmaAllocation allocation) : VKR_buffer_ptr(buffer_handle,
             allocation) {
    }


    std::list<address_and_length> memory_pool;
};

void copy_vk_buffer_and_execution(const VKR_buffer_ptr& srcBuffer, const VKR_buffer_ptr& dstBuffer, VkDeviceSize size);

void end_and_submit_one_command_buffer(VkCommandBuffer commandBuffer);

VkCommandBuffer begin_one_command_buffer();

bool copy_mem_from_cpu_to_gpu(const VKR_buffer_ptr &buffer, const std::function<void(void *)> &mem_copy_callback);

VKR_buffer_ptr create_vma_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaAllocationCreateFlags flags);

void discard_buffer_map_clean();


#endif //HELLO_MAC_VULKAN_BUFFER_H
