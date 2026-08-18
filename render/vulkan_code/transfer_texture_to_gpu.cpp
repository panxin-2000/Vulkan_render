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
        VkImageCreateInfo texImgCI{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType     = VK_IMAGE_TYPE_2D,
            .format        = ktxTexture_GetVkFormat(ktxTexture),
            .extent        = {.width = ktxTexture->baseWidth, .height = ktxTexture->baseWidth, .depth = 1},
            .mipLevels     = ktxTexture->numLevels,
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
            .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .image = image_handle_temp,
            .viewType         = VK_IMAGE_VIEW_TYPE_2D, .format                   = texImgCI.format,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = ktxTexture->numLevels, .layerCount = 1
            }
        };
        VK_CHECK_RESULT_NOT_EXIT(vkCreateImageView(handle.get_device(), &texVewCI, nullptr, &image_view_temp));
        // Upload
        VkBuffer imgSrcBuffer{};
        VmaAllocation imgSrcAllocation{};
        VkBufferCreateInfo imgSrcBufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = (uint32_t) ktxTexture->dataSize,
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
        memcpy(imgSrcBufferPtr, ktxTexture->pData, ktxTexture->dataSize);
        VkFenceCreateInfo fenceOneTimeCI{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        VkFence fenceOneTime{};
        VK_CHECK_RESULT_NOT_EXIT(vkCreateFence(handle.get_device(), &fenceOneTimeCI, nullptr, &fenceOneTime));
        VkCommandBuffer cbOneTime{};
        VkCommandBufferAllocateInfo cbOneTimeAI{
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = Engine::instance().get_command_pool(),
            .commandBufferCount = 1
        };
        VK_CHECK_RESULT_NOT_EXIT(vkAllocateCommandBuffers(handle.get_device(), &cbOneTimeAI, &cbOneTime));
        VkCommandBufferBeginInfo cbOneTimeBI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };
        VK_CHECK_RESULT_NOT_EXIT(vkBeginCommandBuffer(cbOneTime, &cbOneTimeBI));
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
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = ktxTexture->numLevels, .layerCount = 1
            }
        };
        VkDependencyInfo barrierTexInfo{
            .sType                = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrierTexImage
        };
        vkCmdPipelineBarrier2(cbOneTime, &barrierTexInfo);
        std::vector<VkBufferImageCopy> copyRegions{};
        for (auto j = 0; j < ktxTexture->numLevels; j++) {
            ktx_size_t mipOffset{0};
            KTX_error_code ret = ktxTexture_GetImageOffset(ktxTexture, j, 0, 0, &mipOffset);
            copyRegions.push_back({
                                      .bufferOffset = mipOffset,
                                      .imageSubresource{
                                          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = (uint32_t) j,
                                          .layerCount = 1
                                      },
                                      .imageExtent{
                                          .width = ktxTexture->baseWidth >> j, .height = ktxTexture->baseHeight >> j,
                                          .depth = 1
                                      },
                                  });
        }
        vkCmdCopyBufferToImage(cbOneTime, imgSrcBuffer, image_handle_temp, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
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
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = ktxTexture->numLevels, .layerCount = 1
            }
        };
        barrierTexInfo.pImageMemoryBarriers = &barrierTexRead;
        vkCmdPipelineBarrier2(cbOneTime, &barrierTexInfo);
        VK_CHECK_RESULT_NOT_EXIT(vkEndCommandBuffer(cbOneTime));
        VkSubmitInfo oneTimeSI{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1, .pCommandBuffers = &cbOneTime
        }; {
            std::lock_guard<std::mutex> lock(Command_submit_manager::get_mutex());
            VK_CHECK_RESULT_NOT_EXIT(vkQueueSubmit(handle.get_queue(), 1, &oneTimeSI, fenceOneTime));
        }
        // command_submit submit(1, &cbOneTime);
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
            .maxLod           = (float) ktxTexture->numLevels,
        };
        VkSampler sampler = create_vulkan_sample(samplerCI);

        ktxTexture_Destroy(ktxTexture);

        VkDescriptorImageInfo temp{
            .sampler     = sampler,
            .imageView   = image_view_temp,
            .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL
        };
        texture.image       = {image_handle_temp, allocation_temp, image_view_temp};
        texture.sampler     = sampler;
        texture.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
        return texture;
    } else if (ext == ".png" || ext == ".jpg" || ext == ".bmp" || ext == ".tga") {
        return create_2d_texture(filename);
    }
    return {};
}
