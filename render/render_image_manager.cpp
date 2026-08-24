//
// Created by 潘鑫 on 2026/8/15.
//
#include "render_image_manager.h"

#include "engine.h"

void Render_image_manager::using_to_free() {
    for (auto &[_, using_of_free]: map_) {
        for (const auto &use: using_of_free.is_using) {
            using_of_free.is_free.push_back(use);
        }
        using_of_free.is_using.clear();
    }
}

VKR_image_ptr Render_image_manager::get_one_position_image() {
    const VkExtent2D extent = VK_backend::instance().get_swap_rational_extent();
    Image_and_view_parameters parameters{
        .format = VK_FORMAT_R16G16B16A16_SFLOAT,
        .width  = extent.width,
        .height = extent.height,
        .depth  = 1,
        .usage  = static_cast<VkImageUsageFlagBits>(
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
        .aspectMask  = VK_IMAGE_ASPECT_COLOR_BIT,
        .tiling      = VK_IMAGE_TILING_OPTIMAL,
        .mipLevels   = 1,
        .arrayLayers = 1,
        .flags       = 0
    };
    return find(parameters);
}

VKR_image_ptr Render_image_manager::get_one_normal_image() {
    const VkExtent2D extent = VK_backend::instance().get_swap_rational_extent();
    Image_and_view_parameters parameters{
        .format = VK_FORMAT_R16G16B16A16_SFLOAT,
        .width  = extent.width,
        .height = extent.height,
        .depth  = 1,
        .usage  = static_cast<VkImageUsageFlagBits>(
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
        .aspectMask  = VK_IMAGE_ASPECT_COLOR_BIT,
        .tiling      = VK_IMAGE_TILING_OPTIMAL,
        .mipLevels   = 1,
        .arrayLayers = 1,
        .flags       = 0

    };
    return find(parameters);
}

VKR_image_ptr Render_image_manager::get_one_color_image() {
    const VkExtent2D extent = VK_backend::instance().get_swap_rational_extent();
    Image_and_view_parameters parameters{
        .format = VK_FORMAT_B8G8R8A8_SRGB,
        .width  = extent.width,
        .height = extent.height,
        .depth  = 1,
        .usage  = static_cast<VkImageUsageFlagBits>(
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
        .aspectMask  = VK_IMAGE_ASPECT_COLOR_BIT,
        .tiling      = VK_IMAGE_TILING_OPTIMAL,
        .mipLevels   = 1,
        .arrayLayers = 1,
        .flags       = 0

    };
    return find(parameters);
}

VKR_image_ptr Render_image_manager::get_one_entity_image() {
    const VkExtent2D extent = VK_backend::instance().get_swap_rational_extent();
    Image_and_view_parameters parameters{
        .format = VK_FORMAT_R32_UINT,
        .width  = extent.width,
        .height = extent.height,
        .depth  = 1,
        .usage  = static_cast<VkImageUsageFlagBits>(
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
        .aspectMask  = VK_IMAGE_ASPECT_COLOR_BIT,
        .tiling      = VK_IMAGE_TILING_OPTIMAL,
        .mipLevels   = 1,
        .arrayLayers = 1,
        .flags       = 0

    };
    return find(parameters);
}


void Render_image_manager::create() {
}

VKR_image_ptr Render_image_manager::get_one_depth_image() {
    Image_and_view_parameters parameters{};
    const VkExtent2D extent = VK_backend::instance().get_swap_rational_extent();
    parameters.format       = VK_backend::instance().get_depth_format();
    parameters.width        = extent.width;
    parameters.height       = extent.height;
    parameters.depth        = 1;
    parameters.usage        = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    parameters.aspectMask   = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    parameters.tiling       = VK_IMAGE_TILING_OPTIMAL;
    parameters.mipLevels    = 1;
    parameters.arrayLayers  = 1;
    parameters.flags        = 0;


    return find(parameters);
}

VKR_image_ptr Render_image_manager::get_one_depth_AO_image() {
    Image_and_view_parameters parameters{};
    parameters.format = VK_FORMAT_D32_SFLOAT;
    parameters.width  = 2016;
    parameters.height = 1832;
    parameters.depth  = 1;
    parameters.usage  = static_cast<VkImageUsageFlagBits>(
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    parameters.aspectMask  = VK_IMAGE_ASPECT_DEPTH_BIT;
    parameters.tiling      = VK_IMAGE_TILING_OPTIMAL;
    parameters.mipLevels   = 1;
    parameters.arrayLayers = 1;
    parameters.flags       = 0;

    return find(parameters);
}

VKR_image_ptr Render_image_manager::get_one_shadow_image() {
    Image_and_view_parameters parameters{};
    parameters.format = VK_FORMAT_D32_SFLOAT;
    parameters.width  = 2048;
    parameters.height = 2048;
    parameters.depth  = 1;
    parameters.usage  = static_cast<VkImageUsageFlagBits>(
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    parameters.aspectMask  = VK_IMAGE_ASPECT_DEPTH_BIT;
    parameters.tiling      = VK_IMAGE_TILING_OPTIMAL;
    parameters.mipLevels   = 1;
    parameters.arrayLayers = 4; // SHADOW_MAP_CASCADE_COUNT 这里之后需要更改
    parameters.flags       = 0;

    return find(parameters);
}

VKR_image_ptr Render_image_manager::get_one_depth_SSAO_image() {
    Image_and_view_parameters parameters{};
    parameters.format = VK_FORMAT_R8_UNORM;
    parameters.width  = 2016;
    parameters.height = 1832;
    parameters.depth  = 1;
    parameters.usage  = static_cast<VkImageUsageFlagBits>(
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    parameters.aspectMask  = VK_IMAGE_ASPECT_COLOR_BIT;
    parameters.tiling      = VK_IMAGE_TILING_OPTIMAL;
    parameters.mipLevels   = 1;
    parameters.arrayLayers = 1;
    parameters.flags       = 0;

    return find(parameters);
}


VKR_image_ptr Render_image_manager::find(const Image_and_view_parameters &parameters) {
    if (map_.contains(parameters)) {
        auto &using_of_free = map_[parameters];
        if (using_of_free.is_free.empty()) {
            auto result = create_2d_image_and_view(parameters);
            using_of_free.is_using.push_back(result);
            return result;
        }
        const auto result = using_of_free.is_free.back();
        using_of_free.is_free.pop_back();
        using_of_free.is_using.push_back(result);
        return result;
    } else {
        auto result = create_2d_image_and_view(parameters);
        Using_of_Free using_of_free{};
        using_of_free.is_using.push_back(result);
        map_[parameters] = using_of_free;
        return result;
    }
}


void Render_image_manager::destroy() {
    map_.clear();
    // 那么我的问题是, 这两个清理掉之后,时候就没有 共享指针了
}
