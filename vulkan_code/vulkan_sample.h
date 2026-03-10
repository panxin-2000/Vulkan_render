//
// Created by 潘鑫 on 2026/3/10.
//

#ifndef HELLO_MAC_VULKAN_SAMPLE_H
#define HELLO_MAC_VULKAN_SAMPLE_H

#include <volk.h>

VkSampler create_vulkan_sample(VkSamplerCreateInfo &samplerCI);

void destroy_all_vulkan_sample();
#endif //HELLO_MAC_VULKAN_SAMPLE_H
