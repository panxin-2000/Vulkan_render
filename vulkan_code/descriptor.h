//
// Created by 潘鑫 on 2026/1/25.
//

#ifndef HOWTOVULKAN_DESCRIPTOR_H
#define HOWTOVULKAN_DESCRIPTOR_H
#include "vulkan_device_handle.h"
#include "descriptor_set_layout.h"


static inline VkDescriptorSetLayoutBindingFlagsCreateInfo DescriptorSetLayoutBindingFlagsCreateInfo(
    const std::vector<VkDescriptorBindingFlags> &descVariableFlags) {
    const VkDescriptorSetLayoutBindingFlagsCreateInfo descBindingFlags{
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount  = static_cast<uint32_t>(descVariableFlags.size()),
        .pBindingFlags = descVariableFlags.data(),
    };
    return descBindingFlags;
}

/**
 * 两个参数的 vector 的 size 需要一致
 * @param handle
 * @param setLayoutBindings
 * @param descriptor_binding_flags  哪怕 全部填零也是需要一致的
 * @return
 */
static VkDescriptorSetLayout
create_descriptor_set_layout(const VKDevice &handle,
                             const std::vector<VkDescriptorSetLayoutBinding> &setLayoutBindings,
                             const std::vector<VkDescriptorBindingFlags> &descriptor_binding_flags) {
    VkDescriptorSetLayout descriptorSetLayout;

    // descVariableFlags 要么没有，要么需要和 setLayoutBindings 一致
    const auto descBindingFlags = DescriptorSetLayoutBindingFlagsCreateInfo(descriptor_binding_flags);

    const auto descriptorLayout = descriptorSetLayoutCreateInfo(setLayoutBindings, (void *) &descBindingFlags);
    VK_CHECK_RESULT_NOT_EXIT(vkCreateDescriptorSetLayout(handle.get_device(), &descriptorLayout, nullptr, &
                                 descriptorSetLayout));
    return descriptorSetLayout;
}


/**
 * 创建描述符的布局
 * @param handle
 * @param size
 * @return
 */
VkDescriptorSetLayout Create_texture_binding_lessLayout(const VKDevice &handle, uint32_t size) {
    // Descriptor (indexing)
    VkDescriptorSetLayout descriptorSetLayoutTex                      = VK_NULL_HANDLE;
    const std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
        descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                   size),
    };
    const std::vector<VkDescriptorBindingFlags> descVariableFlags{
        VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT,
    };
    // descVariableFlags 要么没有，要么需要和 setLayoutBindings 一致
    const auto descBindingFlags = DescriptorSetLayoutBindingFlagsCreateInfo(descVariableFlags);
    const auto descriptorLayout = descriptorSetLayoutCreateInfo(setLayoutBindings, (void *) &descBindingFlags);
    VK_CHECK_RESULT_NOT_EXIT(vkCreateDescriptorSetLayout(handle.get_device(), &descriptorLayout, nullptr, &
                                 descriptorSetLayoutTex));
    return descriptorSetLayoutTex;
}


/**
 * 更新描述符
 * @param handle
 * @param textureDescriptors
 * @param descriptor_set_texture
 */
void update_descriptor_sets(const VKDevice &handle, std::vector<VkDescriptorImageInfo> textureDescriptors,
                            std::vector<VkDescriptorSet> descriptor_set_texture) {
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


/**
 * 申请描述符
 * @param handle
 * @param size
 * @param descriptorSetLayout
 * @return
 */
std::vector<VkDescriptorSet> AllocateDescriptorSets(VKDevice &handle, uint32_t size,
                                                    VkDescriptorSetLayout descriptorSetLayout) {
    std::vector<VkDescriptorSet> descriptor_set_texture;
    std::vector<uint32_t> variableDescCount{size, size};
    std::vector<VkDescriptorSetLayout> layouts{descriptorSetLayout, descriptorSetLayout};

    // Vulkan 协议强制规定：只有索引号（Binding Number）最大的那一个绑定可以是可变的
    // 位置限制： 只有描述符集布局中 Binding 编号最大 的那个绑定才能设置为可变长度。
    // 上限约束： 你在 pDescriptorCounts 中指定的数值，不能超过你在 VkDescriptorSetLayoutBinding 中定义的 descriptorCount（即最大上限）。
    // 特性开启： 需要在物理设备特性中开启 descriptorIndexing 的相关支持，具体可参考 Vulkan 硬件数据库 检查你的显卡是否支持 runtimeDescriptorArray
    VkDescriptorSetVariableDescriptorCountAllocateInfo variableDescCountAI{
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT,
        .descriptorSetCount = static_cast<uint32_t>(variableDescCount.size()),
        .pDescriptorCounts  = variableDescCount.data(),
    };

    descriptor_set_texture.resize(layouts.size());

    VkDescriptorSetAllocateInfo texDescSetAlloc{
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = &variableDescCountAI,
        .descriptorPool     = handle.get_descriptor_pool(),
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()), // // 打算分配的集合数量
        .pSetLayouts        = layouts.data(),                        // 指向布局数组的指针,长度必须等于 descriptorSetCount
    };
    VK_CHECK_RESULT_NOT_EXIT(vkAllocateDescriptorSets(handle.get_device(), &texDescSetAlloc, descriptor_set_texture.data
                                 ()));
    return descriptor_set_texture;
}


auto create_descriptor_set_layouts(const VKDevice &handle,
                                   const std::array<std::map<uint32_t, ResourceInfo>, max_set> sorted_bindings_array) {
    std::array<VkDescriptorSetLayout, max_set> setLayoutBindings_array;
    for (uint32_t i = 0; i < max_set; i++) {
        const auto &sorted_bindings = sorted_bindings_array[i];
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
        std::vector<VkDescriptorBindingFlags> descriptor_binding_flags;
        for (const auto &[fst, snd]: sorted_bindings) {
            setLayoutBindings.push_back(snd.LayoutBinding);
            descriptor_binding_flags.push_back(snd.flag);
        }
        auto SetLayout             = create_descriptor_set_layout(handle, setLayoutBindings, descriptor_binding_flags);
        setLayoutBindings_array[i] = SetLayout;
    }
    return setLayoutBindings_array;
}


/**
 * 创建管线描述符布局, set = 0 还是 set = 1 需要在这里设置，解析需要更靠前
 * organize_graphics_descriptor_set_layouts  是 organize_graphics_descriptor_set_layouts 作为参数
 * 经由 create_descriptor_set_layouts 得出的结果
 * @param handle
 * @param descriptor_set_layout  layout(set = 0, binding = 0) layout(set = 1, binding = 0)
 * @return
 */
inline VkPipelineLayout create_pipeline_layout(const VKDevice &handle,
                                               std::vector<VkDescriptorSetLayout> descriptor_set_layout) {
    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
    VkPushConstantRange pushConstantRange{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, .size = sizeof(VkDeviceAddress)
    };
    VkPipelineLayoutCreateInfo pipelineLayoutCI{
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = static_cast<uint32_t>(descriptor_set_layout.size()),
        .pSetLayouts            = descriptor_set_layout.data(),
        .pushConstantRangeCount = 1,
        .pPushConstantRanges    = &pushConstantRange
    };
    VK_CHECK_RESULT_NOT_EXIT(vkCreatePipelineLayout(handle.get_device(), &pipelineLayoutCI, nullptr, &pipelineLayout));
    return pipelineLayout;
}


#endif //HOWTOVULKAN_DESCRIPTOR_H
