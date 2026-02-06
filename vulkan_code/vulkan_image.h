//
// Created by 潘鑫 on 2026/1/23.
//

#ifndef HELLO_MAC_VULKAN_IMAGE_H
#define HELLO_MAC_VULKAN_IMAGE_H
#include "vulkan_device_handle.h"
#include "vulkan_global_macro.h"


VkImageView createImageView(const VKDevice &handle,
                            const VkImage image,
                            const VkFormat format,
                            const VkImageAspectFlags aspectFlags);


void copyBufferToImage(const VKDevice &handle, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

void transitionImageLayout(const VKDevice &handle, VkImage image, VkFormat format, VkImageLayout oldLayout,
                           VkImageLayout newLayout);

std::pair<VkImage, VmaAllocation> createImage(VKDevice &handle, uint32_t width, uint32_t height, VkFormat format,
                                              VkImageTiling tiling, VkImageUsageFlags usage);


#endif //HELLO_MAC_VULKAN_IMAGE_H
