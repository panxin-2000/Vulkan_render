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
#include "tinyddsloader.h"
#include "../engine.h"
#include "vulkan_buffer.h"
#include "vulkan_execute_command.h"
#include "vulkan_sample.h"


// std::array<Texture_parameter, 3> textures{};

std::optional<Texture_parameter> create_textures_to_gpu(const std::string &filename) {
    VK_backend &handle             = VK_backend::instance();
    std::filesystem::path filePath = filename;
    std::string ext                = filePath.extension().string();
    if (ext == ".ktx") {
        Texture_parameter texture;
        VkImage image_handle_temp     = VK_NULL_HANDLE;
        VmaAllocation allocation_temp = VK_NULL_HANDLE;
        VkImageView image_view_temp   = VK_NULL_HANDLE;

        ktxTexture *ktxTexture{nullptr};
        ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);

        auto width      = ktxTexture->baseWidth;
        auto height     = ktxTexture->baseHeight;
        auto baseWidth  = ktxTexture->baseWidth;
        auto baseHeight = ktxTexture->baseHeight;
        auto numLevels  = ktxTexture->numLevels;
        auto format     = ktxTexture_GetVkFormat(ktxTexture);
        auto data_size  = ktxTexture->dataSize;
        auto pdata      = ktxTexture->pData;
        std::vector<size_t> mipOffsets;
        mipOffsets.resize(numLevels);
        for (auto j = 0; j < numLevels; j++) {
            KTX_error_code ret = ktxTexture_GetImageOffset(ktxTexture, j, 0, 0, &mipOffsets[j]);
        }

        VkImageCreateInfo texImgCI{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType     = VK_IMAGE_TYPE_2D,
            .format        = format,
            .extent        = {.width = width, .height = height, .depth = 1},
            .mipLevels     = numLevels,
            .arrayLayers   = 1,
            .samples       = VK_SAMPLE_COUNT_1_BIT,
            .tiling        = VK_IMAGE_TILING_OPTIMAL,
            .usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };
        VmaAllocationCreateInfo texImageAllocCI{.usage = VMA_MEMORY_USAGE_AUTO};
        VK_CHECK_RESULT_NOT_EXIT(vmaCreateImage(handle.get_allocator(), &texImgCI, &texImageAllocCI, &image_handle_temp,
                                     &allocation_temp,
                                     nullptr));
        VkImageViewCreateInfo texVewCI{
            .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image            = image_handle_temp,
            .viewType         = VK_IMAGE_VIEW_TYPE_2D,
            .format           = texImgCI.format,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = numLevels, .layerCount = 1
            }
        };
        VK_CHECK_RESULT_NOT_EXIT(vkCreateImageView(handle.get_device(), &texVewCI, nullptr, &image_view_temp));


        auto mem_copy_function = [&](void *dst) {
            memcpy(dst, pdata, data_size);
        };
        const auto staging_buffer = create_image_stage_buffer(data_size, mem_copy_function);


        auto execute_function = [=](const VkCommandBuffer commandBuffer, const uint64_t time_line) {
            VkImageMemoryBarrier2 barrierTexImage{
                .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask     = VK_PIPELINE_STAGE_2_NONE,
                .srcAccessMask    = VK_ACCESS_2_NONE,
                .dstStageMask     = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                .dstAccessMask    = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .image            = image_handle_temp,
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .levelCount = numLevels,
                    .layerCount = 1
                }
            };
            VkDependencyInfo barrierTexInfo{
                .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers    = &barrierTexImage
            };
            vkCmdPipelineBarrier2(commandBuffer, &barrierTexInfo);
            std::vector<VkBufferImageCopy> copyRegions{};
            for (auto j = 0; j < numLevels; j++) {
                copyRegions.push_back({
                                          .bufferOffset = mipOffsets[j],
                                          .imageSubresource{
                                              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = (uint32_t) j,
                                              .layerCount = 1
                                          },
                                          .imageExtent{
                                              .width  = baseWidth >> j,
                                              .height = baseHeight >> j,
                                              .depth  = 1
                                          },
                                      });
            }
            vkCmdCopyBufferToImage(commandBuffer,
                                   staging_buffer->get_buffer_handle(time_line),
                                   image_handle_temp,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   static_cast<uint32_t>(copyRegions.size()), copyRegions.data());
            VkImageMemoryBarrier2 barrierTexRead{
                .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask     = VK_PIPELINE_STAGE_TRANSFER_BIT,
                .srcAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
                .dstStageMask     = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                .dstAccessMask    = VK_ACCESS_SHADER_READ_BIT,
                .oldLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .newLayout        = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
                .image            = image_handle_temp,
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = numLevels, .layerCount = 1
                }
            };
            barrierTexInfo.pImageMemoryBarriers = &barrierTexRead;
            vkCmdPipelineBarrier2(commandBuffer, &barrierTexInfo);
        }; {
            Command_submit_manager::add_execute_function(execute_function);
        }

        // Sampler
        VkSamplerCreateInfo samplerCI{
            .sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter        = VK_FILTER_LINEAR,
            .minFilter        = VK_FILTER_LINEAR,
            .mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy    = 8.0f,
            .maxLod           = (float) numLevels,
        };
        VkSampler sampler = create_vulkan_sample(samplerCI);


        VkDescriptorImageInfo temp{
            .sampler     = sampler,
            .imageView   = image_view_temp,
            .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL
        };
        texture.image       = {image_handle_temp, allocation_temp, image_view_temp};
        texture.sampler     = sampler;
        texture.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

        ktxTexture_Destroy(ktxTexture);
        return texture;
    } else if (ext == ".png" || ext == ".jpg" || ext == ".bmp" || ext == ".tga") {
        return create_2d_texture(filename);
    }
    return {};
}


VkFormat MapDDSFormatToVulkan(tinyddsloader::DDSFile::DXGIFormat dxgiFormat) {
    switch (dxgiFormat) {
        case tinyddsloader::DDSFile::DXGIFormat::BC7_UNorm: return VK_FORMAT_BC7_UNORM_BLOCK;
        case tinyddsloader::DDSFile::DXGIFormat::BC7_UNorm_SRGB: return VK_FORMAT_BC7_SRGB_BLOCK;
        case tinyddsloader::DDSFile::DXGIFormat::BC1_UNorm: return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        case tinyddsloader::DDSFile::DXGIFormat::BC3_UNorm: return VK_FORMAT_BC3_UNORM_BLOCK;
        default: return VK_FORMAT_UNDEFINED;
    }
}

Texture_parameter load_dds_to_gpu(const tinyddsloader::DDSFile &dds) {
    auto width  = dds.GetWidth();;
    auto height = dds.GetHeight();;

    auto image_0    = dds.GetImageData(0);
    auto baseWidth  = image_0->m_width;
    auto baseHeight = image_0->m_height;
    auto numLevels  = dds.GetMipCount();
    auto format     = MapDDSFormatToVulkan(dds.GetFormat()); // VK_FORMAT_BC7_UNORM_BLOCK
    auto data_size  = dds.m_dds.size();
    auto pdata      = dds.m_dds.data();
    std::vector<size_t> mipOffsets;
    mipOffsets.resize(numLevels);
    VkDeviceSize totalSize = 0;
    std::vector<const tinyddsloader::DDSFile::ImageData *> mipDataEntries;
    for (auto j = 0; j < numLevels; j++) {
        const auto *imgData = dds.GetImageData(j);
        mipDataEntries.push_back(imgData);
        mipOffsets.push_back(totalSize);
        totalSize += imgData->m_memSlicePitch; // 对于普通2D纹理，这是当前mip的总大小
    }

    VkImage image_handle_temp     = VK_NULL_HANDLE;
    VmaAllocation allocation_temp = VK_NULL_HANDLE;
    VkImageView image_view_temp   = VK_NULL_HANDLE;
    VK_backend &handle            = VK_backend::instance();
    Texture_parameter texture;


    VkImageCreateInfo texImgCI{
        .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType     = VK_IMAGE_TYPE_2D,
        .format        = format,
        .extent        = {.width = width, .height = height, .depth = 1},
        .mipLevels     = numLevels,
        .arrayLayers   = 1,
        .samples       = VK_SAMPLE_COUNT_1_BIT,
        .tiling        = VK_IMAGE_TILING_OPTIMAL,
        .usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };
    VmaAllocationCreateInfo texImageAllocCI{.usage = VMA_MEMORY_USAGE_AUTO};
    VK_CHECK_RESULT_NOT_EXIT(vmaCreateImage(handle.get_allocator(), &texImgCI, &texImageAllocCI, &image_handle_temp,
                                 &allocation_temp,
                                 nullptr));
    VkImageViewCreateInfo texVewCI{
        .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image            = image_handle_temp,
        .viewType         = VK_IMAGE_VIEW_TYPE_2D,
        .format           = texImgCI.format,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = numLevels, .layerCount = 1
        }
    };
    VK_CHECK_RESULT_NOT_EXIT(vkCreateImageView(handle.get_device(), &texVewCI, nullptr, &image_view_temp));
    // Upload
    VkBuffer imgSrcBuffer{};
    VmaAllocation imgSrcAllocation{};
    VkBufferCreateInfo imgSrcBufferCI{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size  = (uint32_t) data_size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    };
    VmaAllocationCreateInfo imgSrcAllocCI{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    VK_CHECK_RESULT_NOT_EXIT(vmaCreateBuffer(handle.get_allocator(), &imgSrcBufferCI, &imgSrcAllocCI, &imgSrcBuffer
                               , &
                                 imgSrcAllocation,
                                 nullptr));
    void *imgSrcBufferPtr{nullptr};
    VK_CHECK_RESULT_NOT_EXIT(vmaMapMemory(handle.get_allocator(), imgSrcAllocation, &imgSrcBufferPtr));
    // memcpy(imgSrcBufferPtr, pdata, data_size);

    for (uint32_t mip = 0; mip < numLevels; ++mip) {
        uint8_t *destPtr = static_cast<uint8_t *>(imgSrcBufferPtr) + mipOffsets[mip];
        std::memcpy(destPtr, mipDataEntries[mip]->m_mem,
                    mipDataEntries[mip]->m_memSlicePitch);
    }


    VkFenceCreateInfo fenceOneTimeCI{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    VkFence fenceOneTime{};
    VK_CHECK_RESULT_NOT_EXIT(vkCreateFence(handle.get_device(), &fenceOneTimeCI, nullptr, &fenceOneTime));

    auto execute_function = [&](VkCommandBuffer commandBuffer, const uint64_t time_line) {
        VkImageMemoryBarrier2 barrierTexImage{
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask     = VK_PIPELINE_STAGE_2_NONE,
            .srcAccessMask    = VK_ACCESS_2_NONE,
            .dstStageMask     = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .dstAccessMask    = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .image            = image_handle_temp,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = numLevels,
                .layerCount = 1
            }
        };
        VkDependencyInfo barrierTexInfo{
            .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers    = &barrierTexImage
        };
        vkCmdPipelineBarrier2(commandBuffer, &barrierTexInfo);
        std::vector<VkBufferImageCopy> copyRegions{};
        for (auto j = 0; j < numLevels; j++) {
            copyRegions.push_back({
                                      .bufferOffset = mipOffsets[j],
                                      .imageSubresource{
                                          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = (uint32_t) j,
                                          .layerCount = 1
                                      },
                                      .imageExtent{
                                          .width  = mipDataEntries[j]->m_width,
                                          .height = mipDataEntries[j]->m_height,
                                          .depth  = mipDataEntries[j]->m_depth
                                      },
                                  });
        }
        // std::vector<VkBufferImageCopy> bufferCopyRegions(numLevels);
        // for (uint32_t mip = 0; mip < numLevels; ++mip) {
        //     bufferCopyRegions[mip].bufferOffset                    = mipOffsets[mip];
        //     bufferCopyRegions[mip].bufferRowLength                 = 0; // 0 表示紧凑排列
        //     bufferCopyRegions[mip].bufferImageHeight               = 0; // 0 表示紧凑排列
        //     bufferCopyRegions[mip].imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        //     bufferCopyRegions[mip].imageSubresource.mipLevel       = mip;
        //     bufferCopyRegions[mip].imageSubresource.baseArrayLayer = 0;
        //     bufferCopyRegions[mip].imageSubresource.layerCount     = 1;
        //     bufferCopyRegions[mip].imageOffset                     = {0, 0, 0};
        //     bufferCopyRegions[mip].imageExtent                     = {dds.GetWidth(mip), dds.GetHeight(mip), 1};
        // }


        vkCmdCopyBufferToImage(commandBuffer, imgSrcBuffer, image_handle_temp, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               static_cast<uint32_t>(copyRegions.size()), copyRegions.data());
        VkImageMemoryBarrier2 barrierTexRead{
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask     = VK_PIPELINE_STAGE_TRANSFER_BIT,
            .srcAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstStageMask     = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            .dstAccessMask    = VK_ACCESS_SHADER_READ_BIT,
            .oldLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout        = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
            .image            = image_handle_temp,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = numLevels, .layerCount = 1
            }
        };
        barrierTexInfo.pImageMemoryBarriers = &barrierTexRead;
        vkCmdPipelineBarrier2(commandBuffer, &barrierTexInfo);
    }; {
        Command_submit_manager::add_execute_function(execute_function, fenceOneTime);
    }

    VK_CHECK_RESULT_NOT_EXIT(vkWaitForFences(handle.get_device(), 1, &fenceOneTime, VK_TRUE, UINT64_MAX));
    vkDestroyFence(handle.get_device(), fenceOneTime, nullptr);
    vmaUnmapMemory(handle.get_allocator(), imgSrcAllocation);
    vmaDestroyBuffer(handle.get_allocator(), imgSrcBuffer, imgSrcAllocation);

    // Sampler
    VkSamplerCreateInfo samplerCI{
        .sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter        = VK_FILTER_LINEAR,
        .minFilter        = VK_FILTER_LINEAR,
        .mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy    = 8.0f,
        .maxLod           = (float) numLevels,
    };
    VkSampler sampler = create_vulkan_sample(samplerCI);


    VkDescriptorImageInfo temp{
        .sampler     = sampler,
        .imageView   = image_view_temp,
        .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL
    };
    texture.image       = {image_handle_temp, allocation_temp, image_view_temp};
    texture.sampler     = sampler;
    texture.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
    return texture;
}
