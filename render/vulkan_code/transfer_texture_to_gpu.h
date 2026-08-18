//
// Created by 潘鑫 on 2026/1/25.
//

#ifndef HOWTOVULKAN_TRANSFER_TEXTURE_TO_GPU_H
#define HOWTOVULKAN_TRANSFER_TEXTURE_TO_GPU_H

#include <volk.h>

#include "tinyddsloader.h"
#include "vulkan_image.h"


std::optional<Texture_parameter> create_textures_to_gpu(const std::string &filename);

// Texture_parameter load_dds_to_gpu(const tinyddsloader::DDSFile &dds);

#endif //HOWTOVULKAN_TRANSFER_TEXTURE_TO_GPU_H
