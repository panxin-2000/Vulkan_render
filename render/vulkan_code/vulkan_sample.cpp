//
// Created by 潘鑫 on 2026/3/10.
//

#include "vulkan_sample.h"
#include "vulkan_backend.h"
#include <assert.h>


std::vector<VkSampler> vulkan_sample_vector;

VkSampler create_vulkan_sample(VkSamplerCreateInfo &samplerCI) {
    // 很简单，只有16个参数 ， 其实只有一个问题，你是用索引呢？ 还是用其他的呢？
    VkSampler sampler   = VK_NULL_HANDLE;
    const auto &backend = VK_backend::instance();
    // Sampler
    assert(samplerCI.sType == VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);

    VK_CHECK_RESULT_NOT_EXIT(vkCreateSampler(backend.get_device(), &samplerCI, nullptr, &sampler));

    if (sampler != VK_NULL_HANDLE) {
        vulkan_sample_vector.push_back(sampler);
    }
    return sampler;
}


void destroy_all_vulkan_sample() {
    const auto &backend = VK_backend::instance();
    for (const auto &sampler: vulkan_sample_vector) {
        vkDestroySampler(backend.get_device(), sampler, nullptr);
    }
    vulkan_sample_vector.clear();
}

VkSampler base_sample() {
    const auto &backend    = VK_backend::instance();
    VkSampler colorSampler = VK_NULL_HANDLE;
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType         = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.magFilter     = VK_FILTER_NEAREST;
    samplerInfo.minFilter     = VK_FILTER_NEAREST;
    samplerInfo.mipmapMode    = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV  = samplerInfo.addressModeU;
    samplerInfo.addressModeW  = samplerInfo.addressModeU;
    samplerInfo.mipLodBias    = 0.0f;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.minLod        = 0.0f;
    samplerInfo.maxLod        = 1.0f;
    samplerInfo.borderColor   = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    return create_vulkan_sample(samplerInfo);
}


VkSampler create_2d_Texture_Sampler() {
    auto &backend = VK_backend::instance();


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
    vkGetPhysicalDeviceFeatures(backend.get_physical_device(), &supportedFeatures);
    if (supportedFeatures.samplerAnisotropy) {
        samplerInfo.anisotropyEnable = VK_TRUE;
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(backend.get_physical_device(), &properties);
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
    return textureSampler = create_vulkan_sample(samplerInfo);
}

VkSampler create_skybox_Texture_Sampler() {
    auto &backend = VK_backend::instance();


    VkSampler textureSampler;

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter    = VK_FILTER_LINEAR;
    samplerInfo.minFilter    = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // sky_cube VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

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
    vkGetPhysicalDeviceFeatures(backend.get_physical_device(), &supportedFeatures);
    if (supportedFeatures.samplerAnisotropy) {
        samplerInfo.anisotropyEnable = VK_TRUE;
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(backend.get_physical_device(), &properties);
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
    return textureSampler = create_vulkan_sample(samplerInfo);
}
