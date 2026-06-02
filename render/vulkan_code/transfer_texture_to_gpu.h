//
// Created by 潘鑫 on 2026/1/25.
//

#ifndef HOWTOVULKAN_TRANSFER_TEXTURE_TO_GPU_H
#define HOWTOVULKAN_TRANSFER_TEXTURE_TO_GPU_H

#include <volk.h>

#include "vulkan_image.h"


std::optional<Texture_parameter> create_textures_to_gpu(const std::string &filename);


#endif //HOWTOVULKAN_TRANSFER_TEXTURE_TO_GPU_H
