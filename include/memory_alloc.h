//
// Created by 潘鑫 on 2026/10/12.
//

#ifndef HELLO_MAC_MEMORY_ALLOC_H
#define HELLO_MAC_MEMORY_ALLOC_H
#include <map>


struct size_and_status {
    uint64_t size_ = 0;
    // bool status_   = false;
};

struct offset_no_status {
    uint64_t offset_ = 0;
};

struct offset_and_size {
    uint64_t offset_ = 0;
    uint64_t size_   = 0;
};


namespace my_namespace {
    using block_offset = uint64_t;
    using alloc_size   = uint64_t;
#define  block_is_free true
#define  block_is_used false

#define SET_block_is_free(x) ((x) & ~(UINT64_C(1) << 63))
#define SET_block_is_used(x) ((x) | (UINT64_C(1) << 63))

#define CHECK_FREE(x) (((x) & (UINT64_C(1) << 63)) == 0)

    class alloc_memory {
    public:
        explicit alloc_memory(const uint64_t total_size) : total_size_(total_size) {
            offset_and_size_map.clear();
            free_block.clear();
            free_block.insert({
                                  total_size, //  空闲大小
                                  0           //  空闲起始地址
                              });
            offset_and_size_map.insert(
                                       {
                                           0,                              //  空闲起始地址
                                           {SET_block_is_free(total_size)} //空闲大小
                                       }


                                      );
        }

        std::pair<block_offset, alloc_size> alloc(uint64_t request_size) {
            std::lock_guard<std::mutex> lock(buffer_block_mutex);
            if (const auto found_free_block_it = free_block.lower_bound(request_size);
                found_free_block_it != free_block.end()) {
                // it->first 是最接近且满足条件的 size
                // it->second 是对应的偏移量
                auto found_free_block_size   = found_free_block_it->first;
                auto found_free_block_offset = found_free_block_it->second;
                // LOG_DEBUG(g_log(), "find free memory size {} offset {} request_size {} ",
                //           found_free_block_size,
                //           found_free_block_offset,
                //           request_size);
                const auto it_offset = offset_and_size_map.find(found_free_block_offset);
                if (found_free_block_size != request_size) {
                    if (found_free_block_it != free_block.end()) {
                        free_block.insert({
                                              found_free_block_size - request_size,    // 空闲大小
                                              {found_free_block_offset + request_size} //  空闲起始地址
                                          });
                        free_block.erase(found_free_block_it);

                        it_offset->second = {SET_block_is_used(request_size)};
                        offset_and_size_map.insert({
                                                       found_free_block_offset + request_size, //  空闲起始地址
                                                       {
                                                           SET_block_is_free(found_free_block_size - request_size),
                                                           // 空闲大小
                                                       }
                                                   });
                    }
                } else {
                    it_offset->second = {SET_block_is_used(request_size)};
                    free_block.erase(found_free_block_it);
                }
                // LOG_DEBUG(g_log(), "VKR_buffer_block GPU_pool_alloc {}", found_free_block_offset);
                return std::make_pair(found_free_block_offset, request_size);
            } else {
                // LOG_DEBUG(g_log(), "find free memory failed for request_size {} ", request_size);
            }
            return {};
        }

        void free(const uint64_t offset) {
            auto &offset_const_and_size_map = offset_and_size_map;
            auto &size_const_and_offset_map = free_block;

            std::lock_guard<std::mutex> lock(buffer_block_mutex);
            auto it_offset        = offset_const_and_size_map.find(offset);
            auto it_offset_before = it_offset;
            if (it_offset != offset_const_and_size_map.begin()) {
                --it_offset_before;
            }
            auto it_offset_after    = offset_const_and_size_map.lower_bound(offset + 1);
            auto before_node_status = block_is_used;
            if (offset == 0) {
                it_offset_before   = {};
                before_node_status = block_is_used;
            } else if (it_offset_before != offset_const_and_size_map.end()) {
                before_node_status = CHECK_FREE(it_offset_before->second.size_);
            }
            auto after_node_status = block_is_used;
            if (it_offset_after != offset_const_and_size_map.end()) {
                after_node_status = CHECK_FREE(it_offset_after->second.size_);
            }

#define erase_before \
    {\
        auto range = size_const_and_offset_map.equal_range(it_offset_before->second.size_); \
        for (auto it = range.first; it != range.second; ++it) {\
            if (it->second == it_offset_before->first) {\
                size_const_and_offset_map.erase(it); \
                break;\
            }\
        }\
    }

#define erase_after \
    {\
        auto range = size_const_and_offset_map.equal_range(it_offset_after->second.size_);\
        for (auto it = range.first; it != range.second; ++it) {\
            if (it->second == it_offset_after->first) {\
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
            if (it_offset != offset_const_and_size_map.end() &&
                before_node_status == block_is_used &&
                after_node_status == block_is_used) {
                it_offset->second = {SET_block_is_free(it_offset->second.size_)};
                size_const_and_offset_map.insert({SET_block_is_free(it_offset->second.size_), {it_offset->first}});
            }

            // 只有前一个能合并
            // 前一个 更改大小                     找到 前一个 size  删除
            // 当前  删除                         添加合并后的 size 和 offset
            if (it_offset != offset_const_and_size_map.end() &&
                before_node_status == block_is_free &&
                after_node_status == block_is_used) {
                erase_before;
                const auto combination_size   = it_offset->second.size_ + it_offset_before->second.size_;
                const auto combination_offset = it_offset_before->first;
                it_offset_before->second      = {SET_block_is_free(combination_size)};
                offset_const_and_size_map.erase(it_offset);
                size_const_and_offset_map.insert({SET_block_is_free(combination_size), {combination_offset}});
            }

            // 之后后一个能合并
            // 当前 更改大小                       找到 后一个 size  删除
            // 后一个 删除                         添加合并后的 size 和 offset
            if (it_offset != offset_const_and_size_map.end() &&
                before_node_status == block_is_used &&
                after_node_status == block_is_free) {
                erase_after;
                const auto combination_size   = it_offset->second.size_ + it_offset_after->second.size_;
                const auto combination_offset = it_offset->first;
                it_offset->second             = {SET_block_is_free(combination_size)};
                offset_const_and_size_map.erase(it_offset_after);
                size_const_and_offset_map.insert({SET_block_is_free(combination_size), {combination_offset}});
            }
            // 两个都能合并
            // 前一个 更改大小                      找到 前一个 和 后一个 size  删除
            // 当前  删除                          添加合并后的 size 和 offset
            // 后一个 删除
            if (it_offset != offset_const_and_size_map.end() &&
                before_node_status == block_is_free &&
                after_node_status == block_is_free) {
                const auto combination_size = it_offset_before->second.size_ +
                                              it_offset->second.size_ +
                                              it_offset_after->second.size_;
                const auto combination_offset = it_offset_before->first;
                erase_before;
                erase_after;
                it_offset_before->second = {SET_block_is_free(combination_size)};
                offset_const_and_size_map.erase(it_offset);
                offset_const_and_size_map.erase(it_offset_after);
                size_const_and_offset_map.insert({SET_block_is_free(combination_size), combination_offset});
            }
        }

        [[nodiscard]] size_t size() const {
            return free_block.size();
        }

    public:
        uint64_t total_size_ = 0;

    private:
        std::mutex buffer_block_mutex;
        std::map<block_offset, size_and_status> offset_and_size_map;
        std::multimap<uint64_t, block_offset> free_block;
    };


#undef  block_is_free
#undef  block_is_used
}

#endif //HELLO_MAC_MEMORY_ALLOC_H
