//
// Created by 潘鑫 on 2026/3/10.
//

#include "vulkan_sample.h"
#include "vulkan_device_handle.h"
#include <assert.h>


std::vector<VkSampler> vulkan_sample_vector;

VkSampler create_vulkan_sample(VkSamplerCreateInfo &samplerCI) {
    // 很简单，只有16个参数 ， 其实只有一个问题，你是用索引呢？ 还是用其他的呢？
    VkSampler sampler  = VK_NULL_HANDLE;
    const auto &handle = VK_handle::get();
    // Sampler
    assert(samplerCI.sType == VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);

    VK_CHECK_RESULT_NOT_EXIT(vkCreateSampler(handle.get_device(), &samplerCI, nullptr, &sampler));

    if (sampler != VK_NULL_HANDLE) {
        vulkan_sample_vector.push_back(sampler);
    }
    return sampler;
}


void destroy_all_vulkan_sample() {
    const auto &handle = VK_handle::get();
    for (const auto &sampler: vulkan_sample_vector) {
        vkDestroySampler(handle.get_device(), sampler, nullptr);
    }
    vulkan_sample_vector.clear();
}
