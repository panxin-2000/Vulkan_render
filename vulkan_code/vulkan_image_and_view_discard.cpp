//
// Created by 潘鑫 on 2026/3/19.
//

#include <volk.h>
#include <map>

#include "vulkan_backend.h"
#include "vulkan_image.h"

std::map<VkImageView, uint64_t> discard_image_view_map;
std::map<std::pair<VkImage, VmaAllocation>, uint64_t> discard_image_map;


void VKR_image::destroy_image() {
    if (image_view_ != VK_NULL_HANDLE) {
        discard_image_view_map.insert({{image_view_}, timeline_});
        image_view_ = VK_NULL_HANDLE;
    }
    if (image_handle_ != VK_NULL_HANDLE && allocation_ != VK_NULL_HANDLE) {
        discard_image_map.insert({{image_handle_, allocation_}, timeline_});
        image_handle_ = VK_NULL_HANDLE;
        allocation_   = VK_NULL_HANDLE;
    }
}

void discard_image_and_view_map_clean() {
    const auto &backend = VK_backend::get();
    for (auto it = discard_image_view_map.begin(); it != discard_image_view_map.end(); /* 后面不加 ++ */) {
        const auto &[image_view, timeline] = *it;
        LOG_DEBUG(g_log(), "finished timeline {}  , timeline {} ", backend.get_finished_timeline(), timeline);
        if (backend.get_finished_timeline() >= timeline) {
            vkDestroyImageView(backend.get_device(), image_view, nullptr);
            it = discard_image_view_map.erase(it);
        } else {
            ++it;
        }
    }
    for (auto it = discard_image_map.begin(); it != discard_image_map.end(); /* 后面不加 ++ */) {
        const auto &[image, timeline] = *it;
        LOG_DEBUG(g_log(), "finished timeline {}  , timeline {} ", backend.get_finished_timeline(), timeline);
        if (backend.get_finished_timeline() >= timeline) {
            vmaDestroyImage(backend.get_allocator(), image.first, image.second);
            it = discard_image_map.erase(it);
        } else {
            ++it;
        }
    }
}
