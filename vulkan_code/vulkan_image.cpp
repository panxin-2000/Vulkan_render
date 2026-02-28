//
// Created by 潘鑫 on 2026/1/23.
//

#include "vulkan_image.h"

#include "stb_image.h"
#include "vertex_and_buffer_index.h"
#include "vulkan_device_handle.h"
#include "vulkan_buffer.h"

VkImageView createImageView(const VK_handle &handle,
                            const VkImage image,
                            const VkFormat format,
                            const VkImageAspectFlags aspectFlags,
                            uint32_t mipLevels) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image                           = image;
    viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format                          = format;
    viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = mipLevels;
    viewInfo.subresourceRange.aspectMask     = aspectFlags;
    VkImageView imageView;
    if (vkCreateImageView(handle.get_device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view!");
    }
    return imageView;
}

VkImageView create_sky_cube_ImageView(const VK_handle &handle,
                                      const VkImage image,
                                      const VkFormat format,
                                      const VkImageAspectFlags aspectFlags,
                                      uint32_t mipLevels) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image                           = image;
    viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_CUBE;
    viewInfo.format                          = format;
    viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = mipLevels;
    viewInfo.subresourceRange.aspectMask     = aspectFlags;
    VkImageView imageView;
    if (vkCreateImageView(handle.get_device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view!");
    }
    return imageView;
}


uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("failed to find suitable memory type!");
}

std::pair<VkImage, VmaAllocation> create_sky_cube_Image(VK_handle &handle, uint32_t width, uint32_t height,
                                                        uint32_t mipLevels,
                                                        VkFormat format,
                                                        VkImageTiling tiling,
                                                        VkImageUsageFlags usage) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width  = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth  = 1;
    imageInfo.mipLevels     = mipLevels;
    imageInfo.arrayLayers   = 6;
    imageInfo.format        = format;
    imageInfo.tiling        = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage         = usage;
    imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage                   = VMA_MEMORY_USAGE_AUTO; // 让 VMA 自动选最快的显存
    // 对于 Image，通常不需要 HOST_ACCESS，因为我们走 Staging 流程
    // 如果你强制要 CPU 可见，通常只能用 TILING_LINEAR，性能很差

    VkImage image;
    VmaAllocation allocation;
    VmaAllocationInfo resultInfo;
    vmaCreateImage(handle.get_allocator(), &imageInfo, &allocInfo, &image, &allocation, &resultInfo);

    return {image, allocation};
}

std::pair<VkImage, VmaAllocation> createImage(VK_handle &handle, uint32_t width, uint32_t height, uint32_t mipLevels,
                                              VkFormat format,
                                              VkImageTiling tiling,
                                              VkImageUsageFlags usage) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width  = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth  = 1;
    imageInfo.mipLevels     = mipLevels;
    imageInfo.arrayLayers   = 1;
    imageInfo.format        = format;
    imageInfo.tiling        = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage         = usage;
    imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;


    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage                   = VMA_MEMORY_USAGE_AUTO; // 让 VMA 自动选最快的显存
    // 对于 Image，通常不需要 HOST_ACCESS，因为我们走 Staging 流程
    // 如果你强制要 CPU 可见，通常只能用 TILING_LINEAR，性能很差

    VkImage image;
    VmaAllocation allocation;
    VmaAllocationInfo resultInfo;
    vmaCreateImage(handle.get_allocator(), &imageInfo, &allocInfo, &image, &allocation, &resultInfo);

    return {image, allocation};
}

VKR_buffer_ptr create_image_buffer(const VK_handle &handle, VkDeviceSize size,
                               std::function<void(void *)> mem_copy_callback) {
    auto vBuffer =
            create_vma_buffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                              VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                              VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT); // 最差结果 纯显存（DEVICE_LOCAL）
    if (vBuffer->host_visible() == false) {
        LOG_INFO(g_log(), "can find a cpu write memory, only get GPU memory", size);
        auto staging_buffer = create_staging_buffer(handle, size);
        if (staging_buffer->host_visible() == false) {
            LOG_INFO(g_log(), "can find a cpu write memory, allocate size {}", size);
        } else {
            copy_mem_from_cpu_to_gpu(staging_buffer, mem_copy_callback);
            copy_vk_buffer_and_execution(handle, staging_buffer, vBuffer, size);
        }
        staging_buffer->DestroyBuffer();
    } else {
        copy_mem_from_cpu_to_gpu(vBuffer, mem_copy_callback);
    }
    return vBuffer;
}


void transition_image(VK_handle &handle, VkCommandBuffer commandBuffer, VkImage image, uint32_t baseMipLevel,
                      VkImageLayout oldLayout,
                      VkImageLayout newLayout,
                      VkAccessFlags srcAccessMask,
                      VkAccessFlags dstAccessMask,
                      VkPipelineStageFlags srcStageMask,
                      VkPipelineStageFlags dstStageMask) {
    VkImageMemoryBarrier barrier{};
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image                           = image;
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseMipLevel   = baseMipLevel;
    barrier.oldLayout                       = oldLayout;
    barrier.newLayout                       = newLayout;
    barrier.srcAccessMask                   = srcAccessMask;
    barrier.dstAccessMask                   = dstAccessMask;
    vkCmdPipelineBarrier(commandBuffer,
                         srcStageMask, dstStageMask, 0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);
}


void generateMipmaps(VK_handle &handle, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight,
                     uint32_t mipLevels) {
    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(handle.physical_device_, imageFormat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = begin_one_command_buffer(handle);

    int32_t mipWidth  = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; i++) {
        transition_image(handle, commandBuffer, image, i - 1,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                         VK_ACCESS_TRANSFER_WRITE_BIT,
                         VK_ACCESS_TRANSFER_READ_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT);

        VkImageBlit blit{};
        blit.srcOffsets[0]                 = {0, 0, 0};
        blit.srcOffsets[1]                 = {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel       = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount     = 1;
        blit.dstOffsets[0]                 = {0, 0, 0};
        blit.dstOffsets[1]                 = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
        blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel       = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount     = 1;

        vkCmdBlitImage(commandBuffer,
                       image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blit,
                       VK_FILTER_LINEAR);

        transition_image(handle, commandBuffer, image, i - 1,
                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                         VK_ACCESS_TRANSFER_READ_BIT,
                         VK_ACCESS_SHADER_READ_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    transition_image(handle, commandBuffer, image, mipLevels - 1,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                     VK_ACCESS_TRANSFER_WRITE_BIT,
                     VK_ACCESS_SHADER_READ_BIT,
                     VK_PIPELINE_STAGE_TRANSFER_BIT,
                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    end_and_submit_one_command_buffer(handle, commandBuffer);
    commandBuffer = VK_NULL_HANDLE;
}


std::tuple<VkImage, VmaAllocation, VkImageView> createTextureImage(VK_handle &handle, const std::string &picture_path) {
    assert(!picture_path.empty());
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels        = stbi_load(picture_path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    uint32_t mipLevels;
    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }
    VkDeviceMemory stagingBufferMemory;

    auto mem_copy_function = [pixels,imageSize](void *dst) {
        memcpy(dst, pixels, imageSize);
    };

    auto staging_buffer = create_image_buffer(handle, imageSize, mem_copy_function);
    stbi_image_free(pixels);

    auto [textureImage,textureImage_allocation] = createImage(handle,
                                                              texWidth,
                                                              texHeight, mipLevels,
                                                              VK_FORMAT_R8G8B8A8_SRGB,
                                                              VK_IMAGE_TILING_OPTIMAL,
                                                              VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                              VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                              VK_IMAGE_USAGE_SAMPLED_BIT);


    transitionImageLayout(handle, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
    copyBufferToImage(handle, staging_buffer->get_buffer_handle(), textureImage, static_cast<uint32_t>(texWidth),
                      static_cast<uint32_t>(texHeight));
    transitionImageLayout(handle, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);
    staging_buffer->DestroyBuffer();

    generateMipmaps(handle, textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);


    auto texture_view = createImageView(handle, textureImage,
                                        VK_FORMAT_R8G8B8A8_SRGB,
                                        VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);


    return {textureImage, textureImage_allocation, texture_view};
}

void copyBufferToImage(const VK_handle &handle, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = begin_one_command_buffer(handle);

    VkBufferImageCopy region{};
    region.bufferOffset                    = 0;
    region.bufferRowLength                 = 0;
    region.bufferImageHeight               = 0;
    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;
    region.imageOffset                     = {0, 0, 0};
    region.imageExtent                     = {width, height, 1};
    vkCmdCopyBufferToImage(
                           commandBuffer,
                           buffer,
                           image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &region
                          );

    end_and_submit_one_command_buffer(handle, commandBuffer);
    commandBuffer = VK_NULL_HANDLE;
    // 清理 commandBuffer ，但是 no safe ,手动容易忘记
}


inline void transitionImageLayout(const VK_handle &handle, VkImage image, VkFormat format, VkImageLayout oldLayout,
                                  VkImageLayout newLayout, uint32_t mipLevels) {
    VkCommandBuffer commandBuffer = begin_one_command_buffer(handle);

    VkImageMemoryBarrier barrier{};
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout                       = oldLayout;
    barrier.newLayout                       = newLayout;
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                           = image;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = mipLevels;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
               newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask =
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
                         commandBuffer,
                         sourceStage, destinationStage,
                         0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier
                        );

    end_and_submit_one_command_buffer(handle, commandBuffer);
}


VkSampler createTextureSampler(VK_handle &handle) {
    VkSampler textureSampler;

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter    = VK_FILTER_LINEAR;
    samplerInfo.minFilter    = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // sky_cube VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    // // Sampler // how to vulkan 2026 ,参数会稍微少一点
    // VkSamplerCreateInfo samplerCI{
    //     .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    //     .magFilter = VK_FILTER_LINEAR,
    //     .minFilter = VK_FILTER_LINEAR,
    //     .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
    //     .anisotropyEnable = VK_TRUE,
    //     .maxAnisotropy = 8.0f,
    //     .maxLod = (float) ktxTexture->numLevels,
    // };
    // VK_CHECK_RESULT(vkCreateSampler(handle->get_device(), &samplerCI, nullptr, &textures[i].sampler));


    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(handle.physical_device_, &supportedFeatures);
    if (supportedFeatures.samplerAnisotropy) {
        samplerInfo.anisotropyEnable = VK_TRUE;
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(handle.physical_device_, &properties);
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    } else {
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy    = 1;
    }
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable           = VK_FALSE;
    samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod     = 0.0f;
    samplerInfo.maxLod     = VK_LOD_CLAMP_NONE; // todo : why ? 设置为 1000 ，其实本质的意思是没有层级限制
    // mipLodBias 用于在 shader 计算完成之后再进行一个偏移，使画面稍微锐利或者模糊
    if (vkCreateSampler(handle.get_device(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
    return textureSampler;
}


Texture_parameter create_texture_all(VK_handle &handle, const std::string &picture_path) {
    auto [textureImage,textureImage_allocation,texture_view] = createTextureImage(handle, picture_path);
    auto textureSampler                                      = createTextureSampler(handle);
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView   = texture_view;
    imageInfo.sampler     = textureSampler;
    Texture_parameter texture_parameter{
        .allocation  = textureImage_allocation,
        .image       = textureImage,
        .image_view  = texture_view,
        .sampler     = textureSampler,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };
    return texture_parameter;
}
