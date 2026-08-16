//
// Created by 潘鑫 on 2026/3/19.
//

#include <volk.h>
#include <map>

#include "vulkan_backend.h"
#include "vulkan_image.h"

std::map<VkImageView, uint64_t> discard_image_view_map;
std::map<std::pair<VkImage, VmaAllocation>, std::pair<uint64_t, uint32_t> > discard_image_map;


void VKR_image::destroy_image() {
    if (image_view_ != VK_NULL_HANDLE) {
        discard_image_view_map.insert({{image_view_}, timeline_});
        image_view_ = VK_NULL_HANDLE;
    }
    if (image_handle_ != VK_NULL_HANDLE && allocation_ != VK_NULL_HANDLE) {
        discard_image_map.insert({{image_handle_, allocation_}, {timeline_, index_}});
        add_to_free_index(index_); // 删除的时候 顺便释放掉
        image_handle_ = VK_NULL_HANDLE;
        allocation_   = VK_NULL_HANDLE;
    }
}

void discard_image_and_view_map_clean(uint64_t finished_timeline) {
    const auto &backend = VK_backend::instance();
    for (auto it = discard_image_view_map.begin(); it != discard_image_view_map.end(); /* 后面不加 ++ */) {
        const auto &[image_view, timeline] = *it;
        LOG_DEBUG(g_log(), "finished timeline {}  , timeline {} ", finished_timeline, timeline);
        if (finished_timeline >= timeline) {
            vkDestroyImageView(backend.get_device(), image_view, nullptr);
            it = discard_image_view_map.erase(it);
        } else {
            ++it;
        }
    }
    for (auto it = discard_image_map.begin(); it != discard_image_map.end(); /* 后面不加 ++ */) {
        const auto &[image, timeline] = *it;
        LOG_DEBUG(g_log(), "finished timeline {}  , timeline {} ", finished_timeline, timeline.first);
        if (finished_timeline >= timeline.first) {
            vmaDestroyImage(backend.get_allocator(), image.first, image.second);
            it = discard_image_map.erase(it);
        } else {
            ++it;
        }
    }
}
