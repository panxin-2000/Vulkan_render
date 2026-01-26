//
// Created by 潘鑫 on 2026/1/25.
//

#ifndef HOWTOVULKAN_TRANSFER_TEXTURE_TO_GPU_H
#define HOWTOVULKAN_TRANSFER_TEXTURE_TO_GPU_H

#include <volk.h>
#include "vulkan_device_handle.h"


std::vector<VkDescriptorImageInfo> create_textures_to_gpu(VKDevice *handle, VkCommandPool commandPool);

void destroy_texture(VKDevice *handle);

#endif //HOWTOVULKAN_TRANSFER_TEXTURE_TO_GPU_H
