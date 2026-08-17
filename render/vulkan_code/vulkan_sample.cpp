//
// Created by 潘鑫 on 2026/3/10.
//

#include "vulkan_sample.h"
#include "vulkan_backend.h"
#include <assert.h>
#include "absl/container/flat_hash_map.h"
#include "absl/hash/hash.h"


// 必须在全局命名空间中（或者与 VkSamplerCreateInfo 相同的命名空间，即全局）
// 这样 Abseil 才能通过 ADL (Argument-Dependent Lookup) 找到它
template<typename H>
H AbslHashValue(H h, const VkSamplerCreateInfo &info) {
    // 忽略 sType 和 pNext，因为它们是 Vulkan 链表头，不影响采样器状态
    return H::combine(std::move(h),
                      info.flags,
                      info.magFilter,
                      info.minFilter,
                      info.mipmapMode,
                      info.addressModeU,
                      info.addressModeV,
                      info.addressModeW,
                      info.mipLodBias,
                      info.anisotropyEnable,
                      info.maxAnisotropy,
                      info.compareEnable,
                      info.compareOp,
                      info.minLod,
                      info.maxLod,
                      info.borderColor,
                      info.unnormalizedCoordinates
                     );
}


// 为 VkSamplerCreateInfo 实现全局的 == 运算符
inline bool operator==(const VkSamplerCreateInfo &lhs, const VkSamplerCreateInfo &rhs) {
    // 忽略 sType 和 pNext 指针，只比对核心功能字段
    return lhs.flags == rhs.flags &&
           lhs.magFilter == rhs.magFilter &&
           lhs.minFilter == rhs.minFilter &&
           lhs.mipmapMode == rhs.mipmapMode &&
           lhs.addressModeU == rhs.addressModeU &&
           lhs.addressModeV == rhs.addressModeV &&
           lhs.addressModeW == rhs.addressModeW &&
           lhs.mipLodBias == rhs.mipLodBias &&
           lhs.anisotropyEnable == rhs.anisotropyEnable &&
           lhs.maxAnisotropy == rhs.maxAnisotropy &&
           lhs.compareEnable == rhs.compareEnable &&
           lhs.compareOp == rhs.compareOp &&
           lhs.minLod == rhs.minLod &&
           lhs.maxLod == rhs.maxLod &&
           lhs.borderColor == rhs.borderColor &&
           lhs.unnormalizedCoordinates == rhs.unnormalizedCoordinates;
}


absl::flat_hash_map<VkSamplerCreateInfo, VkSampler> map_;

VkSampler create_vulkan_sample(const VkSamplerCreateInfo &samplerCI) {
    if (map_.contains(samplerCI)) {
        return map_[samplerCI];
    } else {
        VkSampler sampler   = VK_NULL_HANDLE;
        const auto &backend = VK_backend::instance();
        // Sampler
        assert(samplerCI.sType == VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);

        VK_CHECK_RESULT_NOT_EXIT(vkCreateSampler(backend.get_device(), &samplerCI, nullptr, &sampler));

        if (sampler != VK_NULL_HANDLE) {
            map_[samplerCI] = sampler;
            return sampler;
        }
    }
    return VK_NULL_HANDLE;
}


void destroy_all_vulkan_sample() {
    const auto &backend = VK_backend::instance();
    for (const auto &[_, sampler]: map_) {
        if (sampler != VK_NULL_HANDLE)
            vkDestroySampler(backend.get_device(), sampler, nullptr);
    }
    map_.clear();
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
    samplerInfo.sType         = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter     = VK_FILTER_NEAREST;
    samplerInfo.minFilter     = VK_FILTER_NEAREST;
    samplerInfo.addressModeU  = VK_SAMPLER_ADDRESS_MODE_REPEAT; // sky_cube VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
    samplerInfo.addressModeV  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.compareEnable = VK_FALSE;

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
