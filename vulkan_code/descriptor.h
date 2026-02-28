//
// Created by 潘鑫 on 2026/1/25.
//

#ifndef HOWTOVULKAN_DESCRIPTOR_H
#define HOWTOVULKAN_DESCRIPTOR_H
#include "vulkan_device_handle.h"
#include "descriptor_organized_sets_and_bindings.h"


// static inline VkDescriptorSetLayoutBindingFlagsCreateInfo DescriptorSetLayoutBindingFlagsCreateInfo(
//     const std::vector<VkDescriptorBindingFlags> &descVariableFlags) {
//     const VkDescriptorSetLayoutBindingFlagsCreateInfo descBindingFlags{
//         .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
//         .bindingCount  = static_cast<uint32_t>(descVariableFlags.size()),
//         .pBindingFlags = descVariableFlags.data(),
//     };
//     return descBindingFlags;
// }


// /**
//  * 创建描述符的布局
//  * @param handle
//  * @param size
//  * @return
//  */
// VkDescriptorSetLayout Create_texture_binding_lessLayout(const VKDevice &handle, uint32_t size) {
//     // Descriptor (indexing)
//     VkDescriptorSetLayout descriptorSetLayoutTex                      = VK_NULL_HANDLE;
//     const std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
//         descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
//                                    size),
//     };
//     const std::vector<VkDescriptorBindingFlags> descVariableFlags{
//         VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT,
//     };
//     // descVariableFlags 要么没有，要么需要和 setLayoutBindings 一致
//     const auto descBindingFlags = DescriptorSetLayoutBindingFlagsCreateInfo(descVariableFlags);
//     const auto descriptorLayout = descriptorSetLayoutCreateInfo(setLayoutBindings, (void *) &descBindingFlags);
//     VK_CHECK_RESULT_NOT_EXIT(vkCreateDescriptorSetLayout(handle.get_device(), &descriptorLayout, nullptr, &
//                                  descriptorSetLayoutTex));
//     return descriptorSetLayoutTex;
// }


/**
 * 更新描述符
 * @param handle
 * @param textureDescriptors
 * @param descriptor_set_texture
 */
inline void update_descriptor_sets(const VK_handle &handle, std::vector<VkDescriptorImageInfo> &textureDescriptors,
                                   const std::vector<VkDescriptorSet> &descriptor_set_texture) {
    std::vector<VkWriteDescriptorSet> writeDescSet;
    for (uint32_t i = 0; i < descriptor_set_texture.size(); i++) {
        VkWriteDescriptorSet temp{
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet          = descriptor_set_texture[i],
            .dstBinding      = 0,
            .descriptorCount = static_cast<uint32_t>(textureDescriptors.size()),
            .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo      = textureDescriptors.data()
        };
        writeDescSet.push_back(temp);
    }

    vkUpdateDescriptorSets(handle.get_device(),
                           writeDescSet.size(),
                           writeDescSet.data(), 0, nullptr);
}


// std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
//     descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0),
//     descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1),
// };

inline auto variable_descriptor(const uint32_t binding_less_size,
                                const std::vector<VkDescriptorBindingFlags> &binding_flags,
                                std::vector<uint32_t> &variableDescCounts) {
    ; // 也应该从一个 vector 传递过来， 然后再根据双缓冲进行翻倍
    //  variableDescCount 中的值如果是零的话，不能访问图片，如果是1 的话，实际上是退化为普通的
    // for (auto &variableDescCount: variableDescCounts) {
    //     variableDescCount = binding_less_size;
    // }
    variableDescCounts.resize(binding_flags.size());

    for (int i = 0; i < binding_flags.size(); i++) {
        if (binding_flags[i] & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) {
            variableDescCounts[i] = binding_less_size;
        } else {
            variableDescCounts[i] = 0;
        }
    }
    // Vulkan 协议强制规定：只有索引号（Binding Number）最大的那一个绑定可以是可变的
    // 位置限制： 只有描述符集布局中 Binding 编号最大 的那个绑定才能设置为可变长度。
    // 上限约束： 你在 pDescriptorCounts 中指定的数值，不能超过你在 VkDescriptorSetLayoutBinding 中定义的 descriptorCount（即最大上限）。
    // 特性开启： 需要在物理设备特性中开启 descriptorIndexing 的相关支持，具体可参考 Vulkan 硬件数据库 检查你的显卡是否支持 runtimeDescriptorArray
    VkDescriptorSetVariableDescriptorCountAllocateInfo variableDescCountAI{
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT,
        .descriptorSetCount = static_cast<uint32_t>(variableDescCounts.size()),
        .pDescriptorCounts  = variableDescCounts.data(),
    };
    return variableDescCountAI;
}


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
inline auto allocate_descriptor_sets(VK_handle &handle,
                                     const std::vector<VkDescriptorSetLayout> &descriptor_set_layouts,
                                     const std::vector<VkDescriptorBindingFlags> *binding_flags) {
    const uint32_t resize_number = descriptor_set_layouts.size();
    std::vector<VkDescriptorSet> descriptor_set_texture;
    if (descriptor_set_layouts.empty())
        return descriptor_set_texture;

    std::vector<uint32_t> variableDescCount;
    descriptor_set_texture.resize(resize_number);

    VkDescriptorSetAllocateInfo texDescSetAlloc{
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = nullptr,
        .descriptorPool     = handle.get_descriptor_pool(),
        .descriptorSetCount = static_cast<uint32_t>(descriptor_set_layouts.size()), // // 打算分配的集合数量
        .pSetLayouts        = descriptor_set_layouts.data(), // 指向布局数组的指针,长度必须等于 descriptorSetCount
    };
    if (binding_flags != nullptr && !binding_flags->empty()) {
        const uint32_t binding_less_size = handle.get_bindless_textures().size();
        auto variableDescCountAI         = variable_descriptor(binding_less_size, *binding_flags, variableDescCount);
        texDescSetAlloc.pNext            = &variableDescCountAI;
    } else {
        texDescSetAlloc.pNext = nullptr;
    }

    VK_CHECK_RESULT_NOT_EXIT(vkAllocateDescriptorSets(handle.get_device(), &texDescSetAlloc,
                                 descriptor_set_texture.data()));
    return descriptor_set_texture;
}


#endif //HOWTOVULKAN_DESCRIPTOR_H
