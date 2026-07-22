//
// Created by 潘鑫 on 2026/1/23.
//

#include "vulkan_buffer.h"

#include "../engine.h"
#include "vulkan_backend.h"
#include "vulkan_execute_command.h"

static std::mutex buffer_block_mutex;


[[nodiscard]] void *VKR_buffer::mapped_address() const {
    const auto &backend = VK_backend::instance();
    VmaAllocationInfo info;
    vmaGetAllocationInfo(backend.get_allocator(), allocation_, &info);
    VkMemoryPropertyFlags props;
    vmaGetMemoryTypeProperties(backend.get_allocator(), info.memoryType, &props);
    if (props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        return info.pMappedData;
    }
    return nullptr;
}


[[nodiscard]] VkDeviceAddress VKR_buffer::get_gpu_device_address() const {
    const auto &backend = VK_backend::instance();
    const VkBufferDeviceAddressInfo vk_buffer_device_address_info{
        .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = buffer_handle_
    };
    const auto deviceAddress = vkGetBufferDeviceAddress(backend.get_device(), &vk_buffer_device_address_info);
    return deviceAddress;
}


[[nodiscard]] bool VKR_buffer::host_visible() const {
    const auto &backend = VK_backend::instance();
    VmaAllocationInfo info;
    vmaGetAllocationInfo(backend.get_allocator(), allocation_, &info);
    VkMemoryPropertyFlags props;
    vmaGetMemoryTypeProperties(backend.get_allocator(), info.memoryType, &props);
    const bool isVisible = props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    return isVisible;
}

VkDeviceSize VKR_buffer::complete_size() const {
    const auto &backend = VK_backend::instance();

    VmaAllocationInfo allocInfo_for_map;
    vmaGetAllocationInfo(backend.get_allocator(), allocation_, &allocInfo_for_map);
    return allocInfo_for_map.size;
}

bool VKR_buffer::flush(const VkDeviceSize offset, VkDeviceSize size) const {
    const auto &backend = VK_backend::instance();
    if (size == 0) {
        VmaAllocationInfo allocInfo_for_map;
        vmaGetAllocationInfo(backend.get_allocator(), allocation_, &allocInfo_for_map);
        size = allocInfo_for_map.size;
    }
    if (need_flush() == true) {
        vmaFlushAllocation(backend.get_allocator(), allocation_, offset, size);
    }
    return true;
}

bool VKR_buffer::unmap_memory() const {
    const auto &backend = VK_backend::instance();
    vmaUnmapMemory(backend.get_allocator(), allocation_);
    return true;
}


void *VKR_buffer::map_memory() const {
    const auto &backend = VK_backend::instance();
    void *bufferPtr     = nullptr;
    vmaMapMemory(backend.get_allocator(), allocation_, &bufferPtr);
    return bufferPtr;
}

bool VKR_buffer::need_flush() const {
    const auto &backend = VK_backend::instance();
    VmaAllocationInfo info;
    vmaGetAllocationInfo(backend.get_allocator(), allocation_, &info);
    VkMemoryPropertyFlags props;
    vmaGetMemoryTypeProperties(backend.get_allocator(), info.memoryType, &props);
    const bool need_flush = (props & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    return !need_flush;
}


[[nodiscard]] VkDeviceAddress get_gpu_device_address(const VkBuffer &buffer) {
    const auto &backend = VK_backend::instance();
    const VkBufferDeviceAddressInfo vk_buffer_device_address_info{
        .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = buffer
    };
    const auto deviceAddress = vkGetBufferDeviceAddress(backend.get_device(), &vk_buffer_device_address_info);
    return deviceAddress;
}


void copy_vk_buffer_and_execution(const VKR_buffer_ptr &srcBuffer,
                                  const VKR_buffer_ptr &dstBuffer, VkDeviceSize size) {
    auto execute_function = [&](const VkCommandBuffer commandBuffer) {
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size      = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer->get_buffer_handle(), dstBuffer->get_buffer_handle(), 1, &copyRegion);
    };

    const temp_command_execute execute;
    execute.add_execute_function(execute_function);
}

static std::mutex queueMutex;

std::mutex &get_vkQueueSubmit_mutex() {
    return queueMutex;
}



bool copy_mem_from_cpu_to_gpu(const VKR_buffer_ptr &buffer,
                              const std::function<void(void *)> &mem_copy_callback) {
    if (buffer->host_visible() == true) {
        if (buffer->map_memory() == nullptr) {
            return false;
        }
        if (mem_copy_callback != nullptr && buffer->mapped_address() != nullptr) {
            mem_copy_callback(buffer->mapped_address());
        }
        buffer->flush();
        buffer->unmap_memory();
        return true;
    } else {
        return false;
    }
}

VKR_buffer_ptr create_vma_buffer(const VkDeviceSize size,
                                 const VkBufferUsageFlags usage, const VmaAllocationCreateFlags flags) {
    VkBuffer buffer                = VK_NULL_HANDLE;
    VmaAllocation allocation       = VK_NULL_HANDLE;
    const VkDeviceSize buffer_size = size;
    const VkBufferCreateInfo BufferCreateInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size  = buffer_size,
        .usage = usage
    };
    const VmaAllocationCreateInfo AllocationCreateInfo{
        .flags = flags,
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    VmaAllocationInfo allocInfo = {};
    const auto &handle          = VK_backend::instance();
    std::lock_guard<std::mutex> lock(buffer_block_mutex);
    VK_CHECK_RESULT_NOT_EXIT(vmaCreateBuffer(handle.get_allocator(),
                                 &BufferCreateInfo, &AllocationCreateInfo,
                                 &buffer, &allocation,
                                 &allocInfo));
    return std::make_shared<VKR_buffer>(buffer, allocation);
}


// 将 timeline 与销毁结合


std::map<std::pair<VkBuffer, VmaAllocation>, uint64_t> discard_buffer_map;


bool VKR_buffer::destroy_buffer() {
    if (buffer_handle_ != VK_NULL_HANDLE && allocation_ != VK_NULL_HANDLE) {
        discard_buffer_map.insert({{buffer_handle_, allocation_}, timeline_});
        buffer_handle_ = VK_NULL_HANDLE;
        allocation_    = VK_NULL_HANDLE;
    }
    return true;
}

VKR_buffer::~VKR_buffer() {
    if (buffer_handle_ != VK_NULL_HANDLE && allocation_ != VK_NULL_HANDLE) {
        std::lock_guard<std::mutex> lock(buffer_block_mutex);
        discard_buffer_map.insert({{buffer_handle_, allocation_}, timeline_});
        buffer_handle_ = VK_NULL_HANDLE;
        allocation_    = VK_NULL_HANDLE;
    }
}


void discard_buffer_block_map_clean();

void discard_buffer_map_clean() {
    const auto &backend = VK_backend::instance();
    discard_buffer_block_map_clean();
    for (auto it = discard_buffer_map.begin(); it != discard_buffer_map.end(); /* 后面不加 ++ */) {
        const auto &[buffer, timeline] = *it;
        LOG_DEBUG(g_log(), "finished timeline {}  , timeline {} ", Engine::instance().get_finished_timeline(),
                  timeline);
        if (Engine::instance().get_finished_timeline() >= timeline) {
            std::lock_guard<std::mutex> lock(buffer_block_mutex);
            vmaDestroyBuffer(backend.get_allocator(), buffer.first, buffer.second);
            it = discard_buffer_map.erase(it);
        } else {
            ++it;
        }
    }
}
