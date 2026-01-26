//
// Created by 潘鑫 on 2026/1/25.
//

#ifndef HOWTOVULKAN_DESCRIPTOR_POOL_H
#define HOWTOVULKAN_DESCRIPTOR_POOL_H
#include "vulkan_device_handle.h"


class Descriptor_Pool {
    VKDevice *handle_;
    uint32_t descriptor_count_;                      //static_cast<uint32_t>(textures.size())
    VkDescriptorPool descriptorPool{VK_NULL_HANDLE}; // 最大的问题就是这里有一个pool


public:
    Descriptor_Pool(VKDevice *handle, const uint32_t descriptor_count_) : handle_(handle),
                                                                          descriptor_count_(descriptor_count_) {
    }

    void init_Descriptor_Pool() {
        VkDescriptorPoolSize poolSize{
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = descriptor_count_
        };
        VkDescriptorPoolCreateInfo descPoolCI{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, .maxSets = 1, .poolSizeCount = 1,
            .pPoolSizes = &poolSize
        };
        chk(vkCreateDescriptorPool(handle_->get_device(), &descPoolCI, nullptr, &descriptorPool));
    }

    VkDescriptorPool &get_pool() {
        return descriptorPool;
    }

    void destroy() {
        vkDestroyDescriptorPool(handle_->get_device(), descriptorPool, nullptr);
    }
};

#endif //HOWTOVULKAN_DESCRIPTOR_POOL_H