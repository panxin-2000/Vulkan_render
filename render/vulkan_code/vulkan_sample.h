//
// Created by 潘鑫 on 2026/3/10.
//

#ifndef HELLO_MAC_VULKAN_SAMPLE_H
#define HELLO_MAC_VULKAN_SAMPLE_H

#include <atomic>
#include <volk.h>


class VKR_Sampler {
    VkSampler sampler = VK_NULL_HANDLE;
    uint32_t index_   = 0;
    static std::atomic<uint32_t> max_index;

public:
    static uint32_t get_one_bindless_index() {
        // uint32_t value;
        // if (free_index.try_dequeue(value) == true) {
        //     return value;
        // }
        const auto return_value = max_index.load();
        ++max_index;
        return return_value;
    }

    [[nodiscard]] VkSampler get_sample() const {
        return sampler;
    }


    VKR_Sampler(const VkSampler sampler, const uint32_t index) : sampler(sampler), index_(index) {
    }
};

VKR_Sampler create_vulkan_sample(const VkSamplerCreateInfo &samplerCI);

void destroy_all_vulkan_sample();

VKR_Sampler create_2d_Texture_Sampler();

VKR_Sampler create_skybox_Texture_Sampler();

VKR_Sampler base_sample();


#endif //HELLO_MAC_VULKAN_SAMPLE_H
