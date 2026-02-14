//
// Created by 潘鑫 on 2026/1/23.
//

#ifndef HELLO_MAC_VULKAN_IMAGE_H
#define HELLO_MAC_VULKAN_IMAGE_H
#include "vulkan_device_handle.h"
#include "vulkan_global_macro.h"


VkImageView createImageView(const VK_handle &handle,
                            const VkImage image,
                            const VkFormat format,
                            const VkImageAspectFlags aspectFlags, uint32_t mipLevels);


void copyBufferToImage(const VK_handle &handle, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

void transitionImageLayout(const VK_handle &handle, VkImage image, VkFormat format, VkImageLayout oldLayout,
                           VkImageLayout newLayout, uint32_t mipLevels);

std::pair<VkImage, VmaAllocation> createImage(VK_handle &handle, uint32_t width, uint32_t height, uint32_t mipLevels,
                                              VkFormat format,
                                              VkImageTiling tiling, VkImageUsageFlags usage);


#endif //HELLO_MAC_VULKAN_IMAGE_H
