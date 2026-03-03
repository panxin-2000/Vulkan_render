//
// Created by 潘鑫 on 2026/1/23.
//

#include "vulkan_buffer.h"
#include "vulkan_device_handle.h"


[[nodiscard]] void *VKR_buffer::mapped_address() const {
    const auto &handle = VK_handle::get();
    VmaAllocationInfo info;
    vmaGetAllocationInfo(handle.get_allocator(), allocation_, &info);
    VkMemoryPropertyFlags props;
    vmaGetMemoryTypeProperties(handle.get_allocator(), info.memoryType, &props);
    if (props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
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

VkDeviceSize VKR_buffer::complete_size() const {
    const auto &handle = VK_handle::get();

    VmaAllocationInfo allocInfo_for_map;
    vmaGetAllocationInfo(handle.get_allocator(), allocation_, &allocInfo_for_map);
    return allocInfo_for_map.size;
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


[[nodiscard]] VkDeviceAddress get_gpu_device_address(const VkBuffer &buffer) {
    const auto &handle = VK_handle::get();
    const VkBufferDeviceAddressInfo vk_buffer_device_address_info{
        .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = buffer
    };
    const auto deviceAddress = vkGetBufferDeviceAddress(handle.get_device(), &vk_buffer_device_address_info);
    return deviceAddress;
}


void copy_vk_buffer_and_execution(const VKR_buffer_ptr &srcBuffer,
                                  const VKR_buffer_ptr &dstBuffer, VkDeviceSize size) {
    const VkCommandBuffer command_buffer = begin_one_command_buffer();
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size      = size;
    vkCmdCopyBuffer(command_buffer, srcBuffer->get_buffer_handle(), dstBuffer->get_buffer_handle(), 1, &copyRegion);

    end_and_submit_one_command_buffer(command_buffer);
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
    return std::make_shared<VKR_buffer>(buffer, allocation);
}


// 将 timeline 与销毁结合

using buffer_offset = VkDeviceSize;

std::map<std::pair<VkBuffer, VmaAllocation>, uint64_t> discard_buffer_map;
std::map<std::pair<VKR_buffer_pool_ptr, buffer_offset>, uint64_t> discard_buffer_block_map;


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
        discard_buffer_map.insert({{buffer_handle_, allocation_}, timeline_});
        buffer_handle_ = VK_NULL_HANDLE;
        allocation_    = VK_NULL_HANDLE;
    }
}


bool VKR_buffer_block::destroy_buffer() {
    if (size_ != 0) {
        discard_buffer_block_map.insert({
                                            {ptr, offset_},
                                            block_timeline_
                                        });
        offset_ = 0;
        size_   = 0;
    }
    return true;
}

VKR_buffer_block::~VKR_buffer_block() {
    //
    if (size_ != 0) {
        discard_buffer_block_map.insert({
                                            {ptr, offset_},
                                            block_timeline_
                                        });
        offset_ = 0;
        size_   = 0;
    }
    LOG_DEBUG(g_log(), "VKR_buffer_block ~~");
};


VKR_buffer_block_ptr GPU_pool_alloc(const VKR_buffer_pool_ptr &buffer, const uint64_t size) {
    auto &offset_and_size_map = buffer->get_offset_and_size_map();
    auto &size_and_offset_map = buffer->get_size_and_offset_map();
    if (const auto freed_memory_it = size_and_offset_map.lower_bound(size);
        freed_memory_it != size_and_offset_map.end()) {
        // it->first 是最接近且满足条件的 size
        // it->second 是对应的偏移量
        std::cout << "找到最合适的块，大小为: " << freed_memory_it->first;
        auto temp_size   = freed_memory_it->first;
        auto temp_offset = freed_memory_it->second;
        if (temp_size != size) {
            auto it_offset = offset_and_size_map.find(temp_offset.offset_);
            if (freed_memory_it != size_and_offset_map.end()) {
                //                                空闲大小              空闲起始地址
                size_and_offset_map.insert({temp_size - size, {temp_offset.offset_ + size}});
                //                                申请大小              申请起始地址
                size_and_offset_map.erase(freed_memory_it);
                // offset 不变           申请大小改变        类型改变
                it_offset->second = {size, false};
                //                                空闲起始地址                  空闲大小
                offset_and_size_map.insert({temp_offset.offset_ + size, {temp_size - size, true}});
            }
        }
        return std::make_shared<VKR_buffer_block>(buffer, temp_offset.offset_, size);
    } else {
        std::cout << "没有足够大的连续空间";
    }
    return {};
}


void GPU_pool_free(const VKR_buffer_pool_ptr &buffer, const uint64_t offset) {
    auto &offset_const_and_size_map = buffer->get_offset_and_size_map();
    auto &size_const_and_offset_map = buffer->get_size_and_offset_map();
    auto it_offset                  = offset_const_and_size_map.find(offset);
    auto it_offset_before           = offset_const_and_size_map.upper_bound(offset - 1);
    auto it_offset_after            = offset_const_and_size_map.lower_bound(offset + 1);
    auto before_bool                = false;
    if (offset == 0) {
        it_offset_before = offset_const_and_size_map.end();
        before_bool      = false;
    } else if (it_offset_before != offset_const_and_size_map.end()) {
        before_bool = it_offset_before->second.status_;
    }
    auto after_bool = false;
    if (it_offset_after != offset_const_and_size_map.end()) {
        after_bool = it_offset_after->second.status_;
    }

#define erase_before \
    {\
        auto range = size_const_and_offset_map.equal_range(it_offset_before->second.size_); \
        for (auto it = range.first; it != range.second; ++it) {\
            if (it->second.offset_ == it_offset_before->first) {\
                size_const_and_offset_map.erase(it); \
                break;\
            }\
        }\
    }

#define erase_after \
    {\
        auto range = size_const_and_offset_map.equal_range(it_offset_after->second.size_);\
        for (auto it = range.first; it != range.second; ++it) {\
            if (it->second.offset_ == it_offset_after->first) {\
                size_const_and_offset_map.erase(it); \
                break;\
            }\
        }\
    }

    // 查找前一个块，检测是否能合并
    // 检测后一个块，检测是否能合并

    // offset_const_and_size_map         size_const_and_offset_map 只有能分配的
    // 两个都不能合并
    // 当前 更改标志位                     添加 size 和 offset
    if (it_offset != offset_const_and_size_map.end() && before_bool == false && after_bool == false) {
        it_offset->second = {it_offset->second.size_, true};
        size_const_and_offset_map.insert({it_offset->second.size_, {it_offset->first}});
    }

    // 只有前一个能合并
    // 前一个 更改大小                     找到 前一个 size  删除
    // 当前  删除                         添加合并后的 size 和 offset
    if (it_offset != offset_const_and_size_map.end() && before_bool == true && after_bool == false) {
        erase_before;
        const auto combination_size   = it_offset->second.size_ + it_offset_before->second.size_;
        const auto combination_offset = it_offset_before->first;
        it_offset_before->second      = {combination_size, true};
        offset_const_and_size_map.erase(it_offset);
        size_const_and_offset_map.insert({combination_size, {combination_offset}});
    }

    // 之后后一个能合并
    // 当前 更改大小                       找到 后一个 size  删除
    // 后一个 删除                         添加合并后的 size 和 offset
    if (it_offset != offset_const_and_size_map.end() && before_bool == false && after_bool == true) {
        erase_after;
        const auto combination_size   = it_offset->second.size_ + it_offset_after->second.size_;
        const auto combination_offset = it_offset->first;
        it_offset->second             = {combination_size, true};
        offset_const_and_size_map.erase(it_offset_after);
        size_const_and_offset_map.insert({combination_size, {combination_offset}});
    }
    // 两个都能合并
    // 前一个 更改大小                      找到 前一个 和 后一个 size  删除
    // 当前  删除                          添加合并后的 size 和 offset
    // 后一个 删除
    if (it_offset != offset_const_and_size_map.end() && before_bool == true && after_bool == true) {
        const auto combination_size = it_offset_before->second.size_ +
                                      it_offset->second.size_ +
                                      it_offset_after->second.size_;
        const auto combination_offset = it_offset_before->first;
        erase_before;
        erase_after;
        it_offset_before->second = {combination_size, true};
        offset_const_and_size_map.erase(it_offset);
        offset_const_and_size_map.erase(it_offset_after);
        size_const_and_offset_map.insert({combination_size, {combination_offset}});
    }
}

void discard_buffer_block_map_clean() {
    const auto &handle = VK_handle::get();
    for (auto it = discard_buffer_block_map.begin(); it != discard_buffer_block_map.end(); /* 后面不加 ++ */) {
        const auto &[buffer, timeline] = *it;
        LOG_DEBUG(g_log(), "finished timeline {}  , timeline {} ", handle.get_finished_timeline(), timeline);
        if (handle.get_finished_timeline() >= timeline) {
            GPU_pool_free(buffer.first, buffer.second);
            it = discard_buffer_block_map.erase(it);
        } else {
            ++it;
        }
    }
}

void discard_buffer_map_clean() {
    const auto &handle = VK_handle::get();
    discard_buffer_block_map_clean();
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
