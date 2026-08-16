//
// Created by 潘鑫 on 2026/3/7.
//
#include "vulkan_buffer.h"
#include "vulkan_backend.h"

std::map<std::pair<VKR_buffer_pool_ptr, buffer_offset>, uint64_t> discard_buffer_block_map;
static std::mutex buffer_block_mutex;


bool VKR_buffer_block::destroy_buffer() {
    if (size_ != 0) {
        std::lock_guard<std::mutex> lock(buffer_block_mutex);
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
        std::lock_guard<std::mutex> lock(buffer_block_mutex);
        discard_buffer_block_map.insert({
                                            {ptr, offset_},
                                            block_timeline_
                                        });
        LOG_DEBUG(g_log(), "VKR_buffer_block add to discard {}  timeline  {}", offset_, block_timeline_);

        offset_ = 0;
        size_   = 0;
    }
};


VKR_buffer_block_ptr GPU_pool_alloc(const VKR_buffer_pool_ptr &buffer, const uint64_t request_size) {
    auto &offset_and_size_map = buffer->get_offset_and_size_map();
    auto &size_and_offset_map = buffer->get_size_and_offset_map();
    std::lock_guard<std::mutex> lock(buffer_block_mutex);
    if (const auto found_free_block_it = size_and_offset_map.lower_bound(request_size);
        found_free_block_it != size_and_offset_map.end()) {
        // it->first 是最接近且满足条件的 size
        // it->second 是对应的偏移量
        auto found_free_block_size   = found_free_block_it->first;
        auto found_free_block_offset = found_free_block_it->second.offset_;
        LOG_DEBUG(g_log(), "find free memory size {} offset {} request_size {} ",
                  found_free_block_size,
                  found_free_block_offset,
                  request_size);
        const auto it_offset = offset_and_size_map.find(found_free_block_offset);
        if (found_free_block_size != request_size) {
            if (found_free_block_it != size_and_offset_map.end()) {
                size_and_offset_map.insert({
                                               found_free_block_size - request_size,    // 空闲大小
                                               {found_free_block_offset + request_size} //  空闲起始地址
                                           });
                size_and_offset_map.erase(found_free_block_it);

                it_offset->second = {request_size, false};
                offset_and_size_map.insert({
                                               found_free_block_offset + request_size, //  空闲起始地址
                                               {
                                                   found_free_block_size - request_size, //空闲大小
                                                   true
                                               }
                                           });
            }
        } else {
            it_offset->second = {request_size, false};
            size_and_offset_map.erase(found_free_block_it);
        }
        LOG_DEBUG(g_log(), "VKR_buffer_block GPU_pool_alloc {}", found_free_block_offset);
        return std::make_shared<VKR_buffer_block>(buffer, found_free_block_offset, request_size);
    } else {
        LOG_DEBUG(g_log(), "find free memory failed for request_size {} ", request_size);
    }
    return {};
}


void GPU_pool_free(const VKR_buffer_pool_ptr &buffer, const uint64_t offset) {
    auto &offset_const_and_size_map = buffer->get_offset_and_size_map();
    auto &size_const_and_offset_map = buffer->get_size_and_offset_map();
    std::lock_guard<std::mutex> lock(buffer_block_mutex);
    auto it_offset        = offset_const_and_size_map.find(offset);
    auto it_offset_before = offset_const_and_size_map.upper_bound(offset - 1);
    auto it_offset_after  = offset_const_and_size_map.lower_bound(offset + 1);
    auto before_bool      = false;
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


void discard_buffer_block_map_clean(uint64_t finished_timeline) {
    const auto &handle             = VK_backend::instance();
    const auto current_finish_time = finished_timeline;
    for (auto it = discard_buffer_block_map.begin(); it != discard_buffer_block_map.end(); /* 后面不加 ++ */) {
        const auto &[buffer, timeline] = *it;
        if (current_finish_time >= timeline + 12) {
            LOG_DEBUG(g_log(), "discard_buffer_block timeline {}  , timeline {} offset {}",
                      current_finish_time,
                      timeline,
                      buffer.second);
            GPU_pool_free(buffer.first, buffer.second);
            it = discard_buffer_block_map.erase(it);
        } else {
            ++it;
        }
    }
}
