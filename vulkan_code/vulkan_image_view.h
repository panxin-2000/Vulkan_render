//
// Created by 潘鑫 on 2026/3/19.
//

#ifndef HELLO_MAC_VULKAN_IMAGE_VIEW_H
#define HELLO_MAC_VULKAN_IMAGE_VIEW_H


#include <volk.h>

VkImageView createImageView(const VkImage image,
                            const VkFormat format,
                            const VkImageAspectFlags aspectFlags,
                            uint32_t mipLevels);


VkImageView create_sky_cube_ImageView(const VkImage image,
                                      const VkFormat format,
                                      const VkImageAspectFlags aspectFlags,
                                      uint32_t mipLevels);

#endif //HELLO_MAC_VULKAN_IMAGE_VIEW_H
