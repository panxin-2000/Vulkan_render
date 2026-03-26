//
// Created by 潘鑫 on 2026/3/27.
//

#include "vulkan_update_descriptor.h"

#include <volk.h>
#include <memory_resource>
#include <vector>

void update_descriptor_sets(std::map<std::string, Update_descriptor_binding> &update_descriptor_sets,
                            const std::vector<DescriptorSet_ptr> &descriptor_sets) {
    const auto &vk_backend = VK_backend::get();
    char stack_memory_pool[1024];
    std::pmr::monotonic_buffer_resource pool{stack_memory_pool, sizeof(stack_memory_pool)};
    std::pmr::polymorphic_allocator<std::byte> alloc{&pool};

    // 可以再次做一个缓存，等到全部的都更新完成之后一次性 vkUpdateDescriptorSets
    std::vector<VkWriteDescriptorSet> descriptor_write_bindings{};


    descriptor_write_bindings.resize(update_descriptor_sets.size());
    size_t i = 0;
    for (auto &[name,binding_update]: update_descriptor_sets) {
        descriptor_write_bindings[i]        = binding_update.descriptor_write_binding;
        descriptor_write_bindings[i].dstSet = descriptor_sets[binding_update.dstSet]->get_descriptor_set();
        if (binding_update.bufferInfo.first) {
            const auto buffer_info = reinterpret_cast<VkDescriptorBufferInfo *>(alloc.
                allocate(sizeof(VkDescriptorBufferInfo)));
            buffer_info->buffer                      = binding_update.bufferInfo.second->get_buffer_handle();
            buffer_info->offset                      = binding_update.bufferInfo.second->offset_;
            buffer_info->range                       = binding_update.bufferInfo.second->size_;
            descriptor_write_bindings[i].pBufferInfo = buffer_info; // 一个需要转换的问题
        } else if (binding_update.texture_info.first) {
            const auto image_info = reinterpret_cast<VkDescriptorImageInfo *>(alloc.
                allocate(sizeof(VkDescriptorImageInfo)));
            *image_info                             = binding_update.texture_info.second.get_descriptor_image_info();
            descriptor_write_bindings[i].pImageInfo = image_info;
        } else if (binding_update.TexelBufferView.first) {
            descriptor_write_bindings[i].pTexelBufferView = &binding_update.TexelBufferView.second;
        }
        ++i;
    }
    vkUpdateDescriptorSets(vk_backend.get_device(),
                           static_cast<uint32_t>(descriptor_write_bindings.size()),
                           descriptor_write_bindings.data(),
                           0,
                           nullptr);
}
