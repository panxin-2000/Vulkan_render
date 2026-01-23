//
// Created by 潘鑫 on 2026/1/23.
//

#ifndef HELLO_MAC_VULKAN_IMAGE_H
#define HELLO_MAC_VULKAN_IMAGE_H
#include "vulkan_global_macro.h"


VkImageView createImageView(const VkDevice device,
                            const VkImage image,
                            const VkFormat format,
                            const VkImageAspectFlags aspectFlags);

void createImage( VkPhysicalDevice physicalDevice, VkDevice device,uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                 VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);

#endif //HELLO_MAC_VULKAN_IMAGE_H
