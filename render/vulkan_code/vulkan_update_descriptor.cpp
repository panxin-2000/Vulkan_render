//
// Created by 潘鑫 on 2026/3/27.
//

#include "vulkan_update_descriptor.h"
#include <volk.h>
#include <memory_resource>
#include <vector>

// 这里 update_descriptor_sets 这个参数总是容易出现问题
void update_descriptor_sets(std::map<std::string, Update_descriptor_binding> &update_descriptor_sets,
                            const std::vector<DescriptorSet_ptr> &descriptor_sets) {
    const auto &vk_backend = VK_backend::instance();
    char stack_memory_pool[1024 * 8];
    std::pmr::monotonic_buffer_resource pool{stack_memory_pool, sizeof(stack_memory_pool)};
    std::pmr::polymorphic_allocator<std::byte> alloc{&pool};

    // 可以再次做一个缓存，等到全部的都更新完成之后一次性 vkUpdateDescriptorSets
    std::vector<VkWriteDescriptorSet> descriptor_write_bindings{};


    descriptor_write_bindings.resize(update_descriptor_sets.size());
    size_t i = 0;
    for (auto &[name,binding_update]: update_descriptor_sets) {
        descriptor_write_bindings[i]        = binding_update.descriptor_write_binding;
        descriptor_write_bindings[i].dstSet = descriptor_sets[binding_update.dstSet]->get_descriptor_set();
        if (descriptor_write_bindings[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
            const auto buffer_info = reinterpret_cast<VkDescriptorBufferInfo *>(alloc.
                allocate(sizeof(VkDescriptorBufferInfo)));
            buffer_info->buffer                      = binding_update.bufferInfo->get_buffer_handle();
            buffer_info->offset                      = binding_update.bufferInfo->offset_;
            buffer_info->range                       = binding_update.bufferInfo->size_;
            descriptor_write_bindings[i].pBufferInfo = buffer_info; // 一个需要转换的问题
        } else if (descriptor_write_bindings[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
            const auto buffer_info = reinterpret_cast<VkDescriptorBufferInfo *>(alloc.
                allocate(sizeof(VkDescriptorBufferInfo)));
            buffer_info->buffer                      = binding_update.SSBO_bufferInfo.get_buffer_handle();
            buffer_info->offset                      = binding_update.SSBO_bufferInfo.get_offset();
            buffer_info->range                       = binding_update.SSBO_bufferInfo.get_size();
            descriptor_write_bindings[i].pBufferInfo = buffer_info; // 一个需要转换的问题
        } else if (descriptor_write_bindings[i].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
                   descriptor_write_bindings[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
                   descriptor_write_bindings[i].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER ||
                   descriptor_write_bindings[i].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) {
            const auto image_info = reinterpret_cast<VkDescriptorImageInfo *>(alloc.
                allocate(sizeof(VkDescriptorImageInfo)));
            *image_info                             = binding_update.texture_info.get_descriptor_image_info();
            descriptor_write_bindings[i].pImageInfo = image_info;
        } else if (descriptor_write_bindings[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ||
                   descriptor_write_bindings[i].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) {
            descriptor_write_bindings[i].pTexelBufferView = &binding_update.TexelBufferView;
        }
        ++i;
    }
    vkUpdateDescriptorSets(vk_backend.get_device(),
                           static_cast<uint32_t>(descriptor_write_bindings.size()),
                           descriptor_write_bindings.data(),
                           0,
                           nullptr);
}

// Validation Error: [ VUID-VkWriteDescriptorSet-descriptorType-00319 ] | MessageID = 0xe7d2bdda
// vkUpdateDescriptorSets(): pDescriptorWrites[0].descriptorType (VK_DESCRIPTOR_TYPE_SAMPLER) is different from pBindings[0].descriptorType (VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) of VkDescriptorSetLayout 0x170000000017.
// The VkDescriptorSetLayout was used to allocate VkDescriptorSet 0x1c000000001c.
// The Vulkan spec states: descriptorType must match the type of dstBinding within dstSet (https://docs.vulkan.org/spec/latest/chapters/descriptorsets.html#VUID-VkWriteDescriptorSet-descriptorType-00319)
// Objects: 1
//     [0] VkDescriptorSetLayout 0x170000000017
//
// Validation Error: [ VUID-VkWriteDescriptorSet-descriptorType-00319 ] | MessageID = 0xe7d2bdda
// vkUpdateDescriptorSets(): pDescriptorWrites[2].descriptorType (VK_DESCRIPTOR_TYPE_SAMPLER) is different from pBindings[0].descriptorType (VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) of VkDescriptorSetLayout 0x170000000017.
// The VkDescriptorSetLayout was used to allocate VkDescriptorSet 0x1c000000001c.
// The Vulkan spec states: descriptorType must match the type of dstBinding within dstSet (https://docs.vulkan.org/spec/latest/chapters/descriptorsets.html#VUID-VkWriteDescriptorSet-descriptorType-00319)
// Objects: 1
//     [0] VkDescriptorSetLayout 0x170000000017
//
// Validation Error: [ VUID-VkWriteDescriptorSet-descriptorType-00319 ] | MessageID = 0xe7d2bdda
// vkUpdateDescriptorSets(): pDescriptorWrites[3].descriptorType (VK_DESCRIPTOR_TYPE_SAMPLER) is different from pBindings[0].descriptorType (VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) of VkDescriptorSetLayout 0x170000000017.
// The VkDescriptorSetLayout was used to allocate VkDescriptorSet 0x1c000000001c.
// The Vulkan spec states: descriptorType must match the type of dstBinding within dstSet (https://docs.vulkan.org/spec/latest/chapters/descriptorsets.html#VUID-VkWriteDescriptorSet-descriptorType-00319)
// Objects: 1
//     [0] VkDescriptorSetLayout 0x170000000017
//
// Validation Error: [ VUID-VkWriteDescriptorSet-descriptorType-00319 ] | MessageID = 0xe7d2bdda
// vkUpdateDescriptorSets(): pDescriptorWrites[5].descriptorType (VK_DESCRIPTOR_TYPE_SAMPLER) is different from pBindings[0].descriptorType (VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) of VkDescriptorSetLayout 0x170000000017.
// The VkDescriptorSetLayout was used to allocate VkDescriptorSet 0x1c000000001c.
// The Vulkan spec states: descriptorType must match the type of dstBinding within dstSet (https://docs.vulkan.org/spec/latest/chapters/descriptorsets.html#VUID-VkWriteDescriptorSet-descriptorType-00319)
// Objects: 1
//     [0] VkDescriptorSetLayout 0x170000000017
