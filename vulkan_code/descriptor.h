//
// Created by 潘鑫 on 2026/1/25.
//

#ifndef HOWTOVULKAN_DESCRIPTOR_H
#define HOWTOVULKAN_DESCRIPTOR_H
#include "vulkan_device_handle.h"


/**
 * 更新描述符
 * @param handle
 * @param textureDescriptors
 * @param descriptor_set_texture
 */
void update_descriptor_sets(const VK_handle &handle, std::vector<VkDescriptorImageInfo> &textureDescriptors,
                            const std::vector<VkDescriptorSet> &descriptor_set_texture);


// std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
//     descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0),
//     descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1),
// };

auto variable_descriptor(const uint32_t binding_less_size,
                         const std::vector<VkDescriptorBindingFlags> &binding_flags,
                         std::vector<uint32_t> &variableDescCounts);


/**
 * 申请描述符，原本输入的是 单个 descriptor_bindings
 * 需要变更为双缓冲或者多缓冲的结果
 * @param handle
 * @param descriptor_set_layouts
 * @param binding_less_size       这个参数不太对，但是我也还没有想好究竟应该如何传入
 * @param descriptor_set_layout   由 glsl 文件描述的单个 set = 0
 * @param binding_flags
 * @return
 */
std::vector<VkDescriptorSet> allocate_descriptor_sets(VK_handle &handle,
                                                      const std::vector<VkDescriptorSetLayout> &descriptor_set_layouts,
                                                      const std::vector<VkDescriptorBindingFlags> &binding_flags = {});


#endif //HOWTOVULKAN_DESCRIPTOR_H
