//
// Created by 潘鑫 on 2026/1/23.
//

#include "vulkan_buffer.h"

#include "vulkan_device_handle.h"
#include "vulkan_image.h"


[[nodiscard]] void *VKR_buffer::mapped_address() const {
    const auto &handle = VK_handle::get();
    VmaAllocationInfo info;
    vmaGetAllocationInfo(handle.get_allocator(), allocation_, &info);
    VkMemoryPropertyFlags props;
    vmaGetMemoryTypeProperties(handle.get_allocator(), info.memoryType, &props);
    const bool isVisible = props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    if (isVisible) {
        return info.pMappedData;
    }
    return nullptr;
}


[[nodiscard]] VkDeviceAddress VKR_buffer::get_gpu_device_address() const {
    const auto &handle = VK_handle::get();
    const VkBufferDeviceAddressInfo vk_buffer_device_address_info{
        .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = buffer_handle_
    };
    const auto deviceAddress = vkGetBufferDeviceAddress(handle.get_device(), &vk_buffer_device_address_info);
    return deviceAddress;
}


[[nodiscard]] bool VKR_buffer::host_visible() const {
    const auto &handle = VK_handle::get();
    VmaAllocationInfo info;
    vmaGetAllocationInfo(handle.get_allocator(), allocation_, &info);
    VkMemoryPropertyFlags props;
    vmaGetMemoryTypeProperties(handle.get_allocator(), info.memoryType, &props);
    const bool isVisible = props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    return isVisible;
}

bool VKR_buffer::flush(const VkDeviceSize offset, VkDeviceSize size) const {
    const auto &handle = VK_handle::get();
    if (size == 0) {
        VmaAllocationInfo allocInfo_for_map;
        vmaGetAllocationInfo(handle.get_allocator(), allocation_, &allocInfo_for_map);
        size = allocInfo_for_map.size;
    }
    if (need_flush() == true) {
        vmaFlushAllocation(handle.get_allocator(), allocation_, offset, size);
    }
    return true;
}

bool VKR_buffer::unmap_memory() const {
    const auto &handle = VK_handle::get();
    vmaUnmapMemory(handle.get_allocator(), allocation_);
    return true;
}


void *VKR_buffer::map_memory() const {
    const auto &handle = VK_handle::get();
    void *bufferPtr    = nullptr;
    vmaMapMemory(handle.get_allocator(), allocation_, &bufferPtr);
    return bufferPtr;
}

bool VKR_buffer::need_flush() const {
    const auto &handle = VK_handle::get();
    VmaAllocationInfo info;
    vmaGetAllocationInfo(handle.get_allocator(), allocation_, &info);
    VkMemoryPropertyFlags props;
    vmaGetMemoryTypeProperties(handle.get_allocator(), info.memoryType, &props);
    const bool need_flush = (props & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    return !need_flush;
}


[[nodiscard]] VkDeviceAddress get_gpu_device_address(const VkBuffer buffer) {
    const auto &handle = VK_handle::get();
    const VkBufferDeviceAddressInfo vk_buffer_device_address_info{
        .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = buffer
    };
    const auto deviceAddress = vkGetBufferDeviceAddress(handle.get_device(), &vk_buffer_device_address_info);
    return deviceAddress;
}


void copy_vk_buffer_and_execution(VKR_buffer_ptr srcBuffer,
                                  VKR_buffer_ptr dstBuffer, VkDeviceSize size) {
    const auto &handle            = VK_handle::get();
    VkCommandBuffer commandBuffer = begin_one_command_buffer();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size      = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer->get_buffer_handle(), dstBuffer->get_buffer_handle(), 1, &copyRegion);

    end_and_submit_one_command_buffer(commandBuffer);
}


void end_and_submit_one_command_buffer(VkCommandBuffer commandBuffer) {
    const auto &handle = VK_handle::get();

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer;

    vkQueueSubmit(handle.get_queue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(handle.get_queue());

    vkFreeCommandBuffers(handle.get_device(), handle.get_command_pool(), 1, &commandBuffer);
}


VkCommandBuffer begin_one_command_buffer() {
    const auto &handle = VK_handle::get();
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool        = handle.get_command_pool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    //  todo : vkAllocateCommandBuffers 必须加锁
    vkAllocateCommandBuffers(handle.get_device(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
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
    const auto &handle          = VK_handle::get();
    VK_CHECK_RESULT_NOT_EXIT(vmaCreateBuffer(handle.get_allocator(),
                                 &BufferCreateInfo, &AllocationCreateInfo,
                                 &buffer, &allocation,
                                 &allocInfo));
    return {buffer, allocation};
}


// 这里是一个需要更改的点，将 timeline 与销毁结合
//


std::map<std::pair<VkBuffer, VmaAllocation>, uint64_t> discard_buffer_map;

bool VKR_buffer::DestroyBuffer() {
    if (buffer_handle_ != VK_NULL_HANDLE && allocation_ != VK_NULL_HANDLE) {
        discard_buffer_map.insert({{buffer_handle_, allocation_}, timeline_});
        buffer_handle_ = VK_NULL_HANDLE;
        allocation_    = VK_NULL_HANDLE;
    }
    return true;
}

VKR_buffer::~VKR_buffer() {
    if (buffer_handle_ != VK_NULL_HANDLE && allocation_ != VK_NULL_HANDLE) {
        discard_buffer_map.insert({{buffer_handle_, allocation_}, timeline_});
        buffer_handle_ = VK_NULL_HANDLE;
        allocation_    = VK_NULL_HANDLE;
    }
}


void discard_buffer_map_clean() {
    const auto &handle = VK_handle::get();
    for (auto it = discard_buffer_map.begin(); it != discard_buffer_map.end(); /* 后面不加 ++ */) {
        const auto &[buffer, timeline] = *it;
        LOG_DEBUG(g_log(), "finished timeline {}  , timeline {} ", handle.get_finished_timeline(), timeline);
        if (handle.get_finished_timeline() >= timeline) {
            vmaDestroyBuffer(handle.get_allocator(), buffer.first, buffer.second);
            it = discard_buffer_map.erase(it);
        } else {
            ++it;
        }
    }
}
