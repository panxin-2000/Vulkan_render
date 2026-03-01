//
// Created by 潘鑫 on 2026/1/23.
//

#ifndef HELLO_MAC_VULKAN_BUFFER_H
#define HELLO_MAC_VULKAN_BUFFER_H
#include <list>
#include "vulkan_global_macro.h"
#include <vk_mem_alloc.h>


class VKR_buffer {
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

    bool DestroyBuffer();

    void *map_memory() const;

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

class VKR_buffer_ptr {
public:
    VKR_buffer_ptr(const VkBuffer buffer_handle,
                   const VmaAllocation allocation) : ptr(std::make_shared<VKR_buffer>(buffer_handle, allocation)) {
    }

    VKR_buffer_ptr() = default;

    ~VKR_buffer_ptr() {
        ptr = nullptr;
    }

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

private:
    std::shared_ptr<VKR_buffer> ptr = nullptr;
};

struct address_and_length {
    uint64_t address = 0;
    uint64_t length  = 0;
    bool if_used     = false;
};

class VKR_buffer_pool : public VKR_buffer_ptr {
public:
    VKR_buffer_pool(const VkBuffer buffer_handle, const VmaAllocation allocation) : VKR_buffer_ptr(buffer_handle,
             allocation) {
    }


    std::list<address_and_length> memory_pool;

    uint64_t alloc_size(const uint64_t size) {
        uint64_t return_address = -1;
        for (auto it = memory_pool.begin(); it != memory_pool.end(); ++it) {
            if (it->if_used == false && it->length == size) {
                it->if_used    = true;
                return_address = it->address;
                break;
            }
            if (it->if_used == false && it->length > size) {
                memory_pool.insert(it, address_and_length{it->address, size, true});
                return_address = it->address;
                it->address    += size;
                it->length     -= size;
                break;
            }
        }
        return return_address;
    }
};

void copy_vk_buffer_and_execution(VKR_buffer_ptr srcBuffer, VKR_buffer_ptr dstBuffer, VkDeviceSize size);

void end_and_submit_one_command_buffer(VkCommandBuffer commandBuffer);

VkCommandBuffer begin_one_command_buffer();

bool copy_mem_from_cpu_to_gpu(const VKR_buffer_ptr &buffer, const std::function<void(void *)> &mem_copy_callback);

VKR_buffer_ptr create_vma_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaAllocationCreateFlags flags);

void discard_buffer_map_clean();


#endif //HELLO_MAC_VULKAN_BUFFER_H
