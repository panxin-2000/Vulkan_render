//
// Created by 潘鑫 on 2026/3/19.
//

#include "vulkan_image_view.h"

#include "vulkan_backend.h"


VkImageView createImageView(const VkImage image,
                            const VkFormat format,
                            const VkImageAspectFlags aspectFlags,
                            uint32_t mipLevels) {
    const auto &backend = VK_backend::get();
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image                           = image;
    viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format                          = format;
    viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = mipLevels;
    viewInfo.subresourceRange.aspectMask     = aspectFlags;
    VkImageView imageView;
    if (vkCreateImageView(backend.get_device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view!");
    }
    return imageView;
}

VkImageView create_sky_cube_ImageView(const VkImage image,
                                      const VkFormat format,
                                      const VkImageAspectFlags aspectFlags,
                                      uint32_t mipLevels) {
    const auto &backend = VK_backend::get();
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image                           = image;
    viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_CUBE;
    viewInfo.format                          = format;
    viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.layerCount     = 6;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.aspectMask     = aspectFlags;
    VkImageView imageView;
    if (vkCreateImageView(backend.get_device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view!");
    }
    return imageView;
}
