//
// Created by 潘鑫 on 2026/3/4.
//

#include "descriptor_pool.h"

#include "vulkan_backend.h"


VkDescriptorPool init_current_descriptor_pool() {
    uint32_t descriptor_count_ = 500; //static_cast<uint32_t>(textures.size())

    const auto &backend             = VK_backend::instance();
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE; // 最大的问题就是这里有一个pool

    static constexpr uint32_t POOL_SIZE_DESCRIPTOR_SETS = 5000;

    static constexpr uint32_t POOL_SIZE_STORAGE_BUFFER         = 1000;
    static constexpr uint32_t POOL_SIZE_STORAGE_IMAGE          = 2500;
    static constexpr uint32_t POOL_SIZE_COMBINED_IMAGE_SAMPLER = 2500;
    static constexpr uint32_t POOL_SIZE_UNIFORM_BUFFER         = 500;
    static constexpr uint32_t POOL_SIZE_UNIFORM_TEXEL_BUFFER   = 100;
    static constexpr uint32_t POOL_SIZE_INPUT_ATTACHMENT       = 100;

    std::vector<VkDescriptorPoolSize> pool_sizes = {
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, POOL_SIZE_STORAGE_BUFFER},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, POOL_SIZE_STORAGE_IMAGE},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, POOL_SIZE_COMBINED_IMAGE_SAMPLER},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, POOL_SIZE_UNIFORM_BUFFER},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, POOL_SIZE_UNIFORM_TEXEL_BUFFER},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, POOL_SIZE_INPUT_ATTACHMENT}
    };


    // 可以参考 blender 中是如何分配的，blender 中有具体的预分配 类型 与 数值
    VkDescriptorPoolCreateInfo descPoolCI{
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags         = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        .maxSets       = POOL_SIZE_DESCRIPTOR_SETS,
        .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
        .pPoolSizes    = pool_sizes.data(),
    };
    VK_CHECK_RESULT(vkCreateDescriptorPool(backend.get_device(), &descPoolCI, nullptr, &descriptorPool));
    return descriptorPool;
}


void destroy_descriptorPool(const VkDescriptorPool &descriptorPool) {
    const auto &backend = VK_backend::instance();
    vkDestroyDescriptorPool(backend.get_device(), descriptorPool, nullptr);
    // 这里只是删除了一个 DescriptorPool ，如果多起来的话，其实是需要多个池子的
}
