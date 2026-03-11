//
// Created by 潘鑫 on 2026/1/25.
//

#ifndef HOWTOVULKAN_TRANSFER_TEXTURE_TO_GPU_H
#define HOWTOVULKAN_TRANSFER_TEXTURE_TO_GPU_H

#include <volk.h>
#include "vulkan_backend.h"


std::optional<Texture_parameter> create_textures_to_gpu(VK_backend &handle, const std::string &filename);

void destroy_texture(VK_backend *handle);

#endif //HOWTOVULKAN_TRANSFER_TEXTURE_TO_GPU_H
