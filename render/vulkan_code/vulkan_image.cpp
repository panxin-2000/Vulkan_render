//
// Created by 潘鑫 on 2026/1/23.
//

#include "vulkan_image.h"
#include "shader_common.h"
#include "stb_image.h"
#include "vertex_and_buffer_index.h"
#include "vulkan_backend.h"
#include "vulkan_buffer.h"
#include "vulkan_image_view.h"
#include "vulkan_sample.h"

std::pair<VkImage, VmaAllocation> create_sky_cube_Image(VK_backend &handle,
                                                        uint32_t width,
                                                        uint32_t height,
                                                        uint32_t mipLevels,
                                                        VkFormat format,
                                                        VkImageTiling tiling,
                                                        VkImageUsageFlags usage) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width  = width;  // cube skybox  width
    imageInfo.extent.height = height; // cube skybox  height  最好相等
    imageInfo.extent.depth  = 1;
    imageInfo.mipLevels     = mipLevels;
    imageInfo.arrayLayers   = 6; // cube skybox
    imageInfo.format        = format;
    imageInfo.tiling        = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage         = usage;
    imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags         = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT; // cube skybox


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

std::pair<VkImage, VmaAllocation> create_2D_Image(uint32_t width,
                                                  uint32_t height,
                                                  uint32_t mipLevels,
                                                  VkFormat format,
                                                  VkImageTiling tiling,
                                                  VkImageUsageFlags usage) {
    const auto &backend = VK_backend::get();
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
    vmaCreateImage(backend.get_allocator(), &imageInfo, &allocInfo, &image, &allocation, &resultInfo);

    return {image, allocation};
}

VKR_buffer_ptr create_image_stage_buffer(const VK_backend &backend, VkDeviceSize size,
                                         std::function<void(void *)> mem_copy_callback) {
    auto vBuffer =
            create_vma_buffer(size, VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT,
                              VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                              VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT); // 最差结果 纯显存（DEVICE_LOCAL）
    if (vBuffer->host_visible() == false) {
        LOG_INFO(g_log(), "can find a cpu write memory, only get GPU memory", size);
        auto staging_buffer = create_staging_buffer(backend, size);
        if (staging_buffer->host_visible() == false) {
            LOG_INFO(g_log(), "can find a cpu write memory, allocate size {}", size);
        } else {
            copy_mem_from_cpu_to_gpu(staging_buffer, mem_copy_callback);
            copy_vk_buffer_and_execution(staging_buffer, vBuffer, size);
        }
        staging_buffer->destroy_buffer();
    } else {
        copy_mem_from_cpu_to_gpu(vBuffer, mem_copy_callback);
    }
    return vBuffer;
}


// todo:: 想起来了，这里写过一次，写的时候还是很头痛的，之后也没有很仔细的验证结果，应该是好了的
void transition_image(VK_backend &handle, VkCommandBuffer commandBuffer, VkImage image, uint32_t baseMipLevel,
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


void generateMipmaps(VK_backend &handle, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight,
                     uint32_t mipLevels) {
    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(handle.get_physical_device(), imageFormat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = begin_one_command_buffer();

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

    end_and_submit_one_command_buffer(commandBuffer);
    commandBuffer = VK_NULL_HANDLE;
}


VKR_image_ptr createTextureImage_detail(VK_backend &handle,
                                        const Picture_parameters &picture_parameters,
                                        const bool have_mip = false) {
    const VkDeviceSize imageSize = picture_parameters.width * picture_parameters.height * picture_parameters.channels;

    uint32_t mipLevels;
    if (have_mip == false) {
        mipLevels = 1;
    } else {
        // 不想创建时可以设置为 1 ，不能设置为零
        mipLevels =
                static_cast<uint32_t>(std::floor(
                                                 std::log2(std::max(picture_parameters.width,
                                                                    picture_parameters.height)))) + 1;
    }

    if (picture_parameters.image_data == nullptr) {
        return {};
        // throw std::runtime_error("failed to load texture image!");
    }
    VkDeviceMemory stagingBufferMemory;

    auto mem_copy_function = [picture_parameters](void *dst) {
        const VkDeviceSize image_size = picture_parameters.width *
                                        picture_parameters.height *
                                        picture_parameters.channels;
        memcpy(dst, picture_parameters.image_data, image_size);
    };

    const auto staging_buffer = create_image_stage_buffer(handle, imageSize, mem_copy_function);

    auto [textureImage,textureImage_allocation] = create_2D_Image(picture_parameters.width,
                                                                  picture_parameters.height,
                                                                  mipLevels,
                                                                  VK_FORMAT_R8G8B8A8_UNORM,
                                                                  VK_IMAGE_TILING_OPTIMAL,
                                                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                                  VK_IMAGE_USAGE_SAMPLED_BIT);


    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
    copyBufferToImage(staging_buffer->get_buffer_handle(), textureImage,
                      static_cast<uint32_t>(picture_parameters.width),
                      static_cast<uint32_t>(picture_parameters.height));
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);
    staging_buffer->destroy_buffer();

    if (mipLevels > 1)
        generateMipmaps(handle, textureImage, VK_FORMAT_R8G8B8A8_UNORM, picture_parameters.width,
                        picture_parameters.height,
                        mipLevels);


    auto texture_view = createImageView(textureImage,
                                        VK_FORMAT_R8G8B8A8_UNORM,
                                        VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);


    return {textureImage, textureImage_allocation, texture_view};
}


VKR_image_ptr createTextureImage(VK_backend &handle, const std::string &picture_path) {
    assert(!picture_path.empty());
    Picture_parameters picture_parameters{};
    picture_parameters.image_data = stbi_load(picture_path.c_str(),
                                              &picture_parameters.width,
                                              &picture_parameters.height,
                                              &picture_parameters.channels, STBI_rgb_alpha);
    auto result = createTextureImage_detail(handle, picture_parameters);

    stbi_image_free(picture_parameters.image_data);
    picture_parameters.image_data = nullptr;
    return result;
}

void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, int layerCount) {
    const auto &handle            = VK_backend::get();
    VkCommandBuffer commandBuffer = begin_one_command_buffer();

    std::vector<VkBufferImageCopy> regions;
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
    for (int i = 0; i < layerCount; i++) {
        region.bufferOffset                    = width * height * 4 * i;
        region.imageSubresource.baseArrayLayer = i;
        regions.emplace_back(region);
    }
    vkCmdCopyBufferToImage(
                           commandBuffer,
                           buffer,
                           image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           regions.size(),
                           regions.data()
                          );

    end_and_submit_one_command_buffer(commandBuffer);
    commandBuffer = VK_NULL_HANDLE;
    // 清理 commandBuffer ，但是 no safe ,手动容易忘记
}


inline void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
                                  VkImageLayout newLayout, uint32_t mipLevels) {
    const auto &handle            = VK_backend::get();
    VkCommandBuffer commandBuffer = begin_one_command_buffer();

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
    // 暂时不动它了，

    end_and_submit_one_command_buffer(commandBuffer);
}

inline void transitionImageLayout_box(VkImage image, VkFormat format, VkImageLayout oldLayout,
                                      VkImageLayout newLayout, uint32_t mipLevels) {
    const auto &handle            = VK_backend::get();
    VkCommandBuffer commandBuffer = begin_one_command_buffer();

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
    // 暂时不动它了，

    end_and_submit_one_command_buffer(commandBuffer);
}

VKR_image_ptr create_skybox_texture(std::vector<Picture_parameters> &picture_parameters) {
    if (picture_parameters.size() < 6) {
        return {};
    }

    auto &handle                 = VK_backend::get();
    const bool have_mip          = false;
    const VkDeviceSize imageSize = picture_parameters[0].width *
                                   picture_parameters[0].height *
                                   picture_parameters[0].channels * picture_parameters.size();

    uint32_t mipLevels;
    if (have_mip == false) {
        mipLevels = 1;
    } else {
        // 不想创建时可以设置为 1 ，不能设置为零
        mipLevels =
                static_cast<uint32_t>(std::floor(
                                                 std::log2(std::max(picture_parameters[0].width,
                                                                    picture_parameters[0].height)))) + 1;
    }

    for (const auto &picture_parameter: picture_parameters) {
        if (picture_parameter.image_data == nullptr) {
            return {};
            // throw std::runtime_error("failed to load texture image!");
        }
    }

    VkDeviceMemory stagingBufferMemory;

    auto mem_copy_function = [picture_parameters](void *dst) {
        for (const auto &picture_parameter: picture_parameters) {
            const VkDeviceSize image_size = picture_parameter.width *
                                            picture_parameter.height *
                                            picture_parameter.channels;
            memcpy(dst, picture_parameter.image_data, image_size);
            dst = static_cast<char *>(dst) + image_size;
        }
    };
    const auto staging_buffer = create_image_stage_buffer(handle, imageSize, mem_copy_function);


    auto [textureImage , textureImage_allocation] = create_sky_cube_Image(handle,
                                                                          picture_parameters[0].width,
                                                                          picture_parameters[0].height,
                                                                          1,
                                                                          VK_FORMAT_R8G8B8A8_SRGB,
                                                                          VK_IMAGE_TILING_OPTIMAL,
                                                                          VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                                          VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                                          VK_IMAGE_USAGE_SAMPLED_BIT);

    transitionImageLayout_box(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6);
    copyBufferToImage(staging_buffer->get_buffer_handle(), textureImage,
                      static_cast<uint32_t>(picture_parameters[0].width),
                      static_cast<uint32_t>(picture_parameters[0].height), 6);

    transitionImageLayout_box(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 6);
    staging_buffer->destroy_buffer();


    auto texture_view = create_sky_cube_ImageView(textureImage,
                                                  VK_FORMAT_R8G8B8A8_SRGB,
                                                  VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);


    return {textureImage, textureImage_allocation, texture_view};
}


Texture_parameter create_2d_texture(const std::string &picture_path) {
    Picture_parameters picture_parameters{};
    picture_parameters.image_data = stbi_load(picture_path.c_str(),
                                              &picture_parameters.width,
                                              &picture_parameters.height,
                                              &picture_parameters.channels, STBI_rgb_alpha);
    picture_parameters.channels = 4;
    auto result                 = create_2d_texture(picture_parameters);
    stbi_image_free(picture_parameters.image_data);
    picture_parameters.image_data = nullptr;
    return result;
}

Texture_parameter create_skybox_texture_all(const std::string &picture_path) {
    auto &handle = VK_backend::get();
    std::vector<std::string> paths;
    paths.push_back("assets/skybox_right.jpg");
    paths.push_back("assets/skybox_left.jpg");
    paths.push_back("assets/skybox_top.jpg");
    paths.push_back("assets/skybox_bottom.jpg");
    paths.push_back("assets/skybox_front.jpg");
    paths.push_back("assets/skybox_back.jpg");

    std::vector<Picture_parameters> picture_parameters_vector;

    for (const auto &picture_path: paths) {
        Picture_parameters picture_parameters{};
        picture_parameters.image_data = stbi_load(picture_path.c_str(),
                                                  &picture_parameters.width,
                                                  &picture_parameters.height,
                                                  &picture_parameters.channels, STBI_rgb_alpha);
        picture_parameters.channels = 4;
        picture_parameters_vector.emplace_back(picture_parameters);
    }


    auto image_ptr = create_skybox_texture(picture_parameters_vector);


    auto textureSampler = create_skybox_Texture_Sampler();
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView   = image_ptr->get_image_view();
    imageInfo.sampler     = textureSampler;
    Texture_parameter texture_parameter{
        .image       = image_ptr,
        .sampler     = textureSampler,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };


    for (const auto &picture_parameters: picture_parameters_vector) {
        stbi_image_free(picture_parameters.image_data);
    }
    picture_parameters_vector.clear();
    return texture_parameter;
}


Texture_parameter create_2d_texture(const Picture_parameters &picture_parameters) {
    auto &handle   = VK_backend::get();
    auto image_ptr = createTextureImage_detail(handle, picture_parameters);

    auto textureSampler = create_2d_Texture_Sampler();
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView   = image_ptr->get_image_view();
    imageInfo.sampler     = textureSampler;
    Texture_parameter texture_parameter{
        .image       = image_ptr,
        .sampler     = textureSampler,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };
    return texture_parameter;
}
