//
// Created by 潘鑫 on 2026/1/25.
//

#include "transfer_texture_to_gpu.h"
#include <vector>

#include <ktx.h>
#include <ktxvulkan.h>
#include <vk_mem_alloc.h>
#include "vulkan_backend.h"
#include <iostream>

#include "create_texture.h"
#include "../engine.h"
#include "vulkan_buffer.h"
#include "vulkan_execute_command.h"
#include "vulkan_sample.h"
#include "fastgltf/types.hpp"


// std::array<Texture_parameter, 3> textures{};


struct KtxTextureDeleter {
    void operator()(ktxTexture *texture) const {
        if (texture) {
            ktxTexture_Destroy(texture); // 确保调用 libktx 的销毁逻辑
        }
    }
};


void transfer_ktx_to_gpu(const VKR_image_ptr image_ptr,
                         const VKR_buffer_ptr staging_buffer,
                         const Image_and_view_parameters &parameters,
                         const std::vector<VkBufferImageCopy> &copyRegions) {
    auto execute_function = [=](const VkCommandBuffer commandBuffer, const uint64_t time_line) {
        VkImageMemoryBarrier2 barrierTexImage{
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask     = VK_PIPELINE_STAGE_2_NONE,
            .srcAccessMask    = VK_ACCESS_2_NONE,
            .dstStageMask     = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .dstAccessMask    = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .image            = image_ptr->get_image_handle(),
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = parameters.mipLevels,
                .layerCount = 1
            }
        };
        VkDependencyInfo barrierTexInfo{
            .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers    = &barrierTexImage
        };
        vkCmdPipelineBarrier2(commandBuffer, &barrierTexInfo);

        vkCmdCopyBufferToImage(commandBuffer,
                               staging_buffer->get_buffer_handle(time_line),
                               image_ptr->get_image_handle(time_line),
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               static_cast<uint32_t>(copyRegions.size()),
                               copyRegions.data());


        VkImageMemoryBarrier2 barrierTexRead{
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask     = VK_PIPELINE_STAGE_TRANSFER_BIT,
            .srcAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstStageMask     = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            .dstAccessMask    = VK_ACCESS_SHADER_READ_BIT,
            .oldLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout        = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
            .image            = image_ptr->get_image_handle(),
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = parameters.mipLevels,
                .layerCount = 1
            }
        };
        barrierTexInfo.pImageMemoryBarriers = &barrierTexRead;
        vkCmdPipelineBarrier2(commandBuffer, &barrierTexInfo);
    };
    Command_submit_manager::add_execute_function(execute_function);
}


VKR_image_ptr load_ktx(ktxTexture *ktxTexture) {
    auto ktx2 = reinterpret_cast<ktxTexture2 *>(ktxTexture);

    // 1. 检查是否需要转码（如果已经是常规格式则不转）
    if (ktxTexture2_NeedsTranscoding(ktx2)) {
        ktx_transcode_fmt_e targetFormat = KTX_TTF_BC7_RGBA;
        KTX_error_code result            = ktxTexture2_TranscodeBasis(ktx2, targetFormat, 0);
        if (result != KTX_SUCCESS) {
            return {};
        }
    }
    auto format = ktxTexture_GetVkFormat(ktxTexture);

    Image_and_view_parameters parameters{
        .format = format,
        .width  = ktxTexture->baseWidth,
        .height = ktxTexture->baseHeight,
        .depth  = ktxTexture->baseDepth,
        .usage  = static_cast<VkImageUsageFlagBits>(
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
        .aspectMask  = VK_IMAGE_ASPECT_COLOR_BIT,
        .tiling      = VK_IMAGE_TILING_OPTIMAL,
        .mipLevels   = ktxTexture->numLevels,
        .arrayLayers = 1,
        .flags       = 0
    };
    auto image_ptr         = create_2d_image_and_view(parameters);
    auto mem_copy_function = [&](void *dst) {
        memcpy(dst, ktxTexture->pData, ktxTexture->dataSize);
    };
    const auto staging_buffer = create_image_stage_buffer(ktxTexture->dataSize, mem_copy_function);

    std::vector<VkBufferImageCopy> copyRegions{};
    for (auto j = 0; j < parameters.mipLevels; j++) {
        ktx_size_t mipOffset{0};
        KTX_error_code ret = ktxTexture_GetImageOffset(ktxTexture, j, 0, 0, &mipOffset);
        copyRegions.push_back({
                                  .bufferOffset = mipOffset,
                                  .imageSubresource{
                                      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = (uint32_t) j,
                                      .layerCount = 1
                                  },
                                  .imageExtent{
                                      .width  = parameters.width >> j,
                                      .height = parameters.height >> j,
                                      .depth  = 1
                                  },
                              });
    }


    transfer_ktx_to_gpu(image_ptr, staging_buffer, parameters, copyRegions);
    return image_ptr;
}


std::optional<Texture_parameter> create_textures_to_gpu(const std::string &filename) {
    std::filesystem::path filePath = filename;
    std::string ext                = filePath.extension().string();
    if (ext == ".ktx") {
        Texture_parameter texture;

        ktxTexture *ktxTexture{nullptr};
        ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);


        // Upload

        auto image_ptr = load_ktx(ktxTexture);


        // Sampler
        VkSamplerCreateInfo samplerCI{
            .sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter        = VK_FILTER_LINEAR,
            .minFilter        = VK_FILTER_LINEAR,
            .mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy    = 8.0f,
            .maxLod           = VK_LOD_CLAMP_NONE,
        };
        VkSampler sampler = create_vulkan_sample(samplerCI);

        ktxTexture_Destroy(ktxTexture);

        texture.image       = image_ptr;
        texture.sampler     = sampler;
        texture.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
        return texture;
    } else if (ext == ".png" || ext == ".jpg" || ext == ".bmp" || ext == ".tga") {
        return create_2d_texture(filename);
    }
    return {};
}
