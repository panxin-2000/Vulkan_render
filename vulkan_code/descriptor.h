//
// Created by 潘鑫 on 2026/1/25.
//

#ifndef HOWTOVULKAN_DESCRIPTOR_H
#define HOWTOVULKAN_DESCRIPTOR_H
#include "vulkan_device_handle.h"
#include "descriptor_pool.h"


class Descriptor {
    VKDevice *handle;
    Descriptor_Pool *descriptor_pool_;

    VkDescriptorSet descriptor_set_texture{VK_NULL_HANDLE};
    VkDescriptorSetLayout descriptorSetLayoutTex{VK_NULL_HANDLE};

public:
    Descriptor(VKDevice *handle, Descriptor_Pool *descriptor_pool) : handle(handle),
                                                                     descriptor_pool_(descriptor_pool) {
    }


    const VkDescriptorSet &get_descriptor_set_texture() {
        return descriptor_set_texture;
    }

    /**
     *
     * @param type             VkDescriptorType    uniform sampler2D 或 uniform 相关
     * @param stageFlags       VK_SHADER_STAGE_VERTEX_BIT  VK_SHADER_STAGE_FRAGMENT_BIT
     * @param binding          layout (binding = 1)   layout (binding = 2)  layout (binding = 3)
     * @param descriptorCount  []中的数量，layout (set = 0, binding = 0) uniform sampler2D samplerColorMap[];
     * @return
     */
    static inline VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
        const VkDescriptorType type,
        const VkShaderStageFlags stageFlags,
        const uint32_t binding,
        const uint32_t descriptorCount = 1) {
        VkDescriptorSetLayoutBinding setLayoutBinding{};
        setLayoutBinding.descriptorType  = type;
        setLayoutBinding.stageFlags      = stageFlags;
        setLayoutBinding.binding         = binding;
        setLayoutBinding.descriptorCount = descriptorCount;
        return setLayoutBinding;
    }

    static inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
        const std::vector<VkDescriptorSetLayoutBinding> &bindings, void *pNext = nullptr) {
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfo.pBindings    = bindings.data();
        descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        descriptorSetLayoutCreateInfo.pNext        = pNext; // 新增加的一行
        return descriptorSetLayoutCreateInfo;
    }

    // static inline VkDescriptorBindingFlags DescriptorBindingFlags(const VkDescriptorBindingFlagBits flag_bits) {
    //     const VkDescriptorBindingFlags descVariableFlag{static_cast<VkDescriptorBindingFlags>(flag_bits)};
    //     return descVariableFlag;
    // }

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
     *
     * @param size   static_cast<uint32_t>(textures.size())
     */
    void CreateDescriptorSetLayout(uint32_t size) {
        // Descriptor (indexing)
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
        VK_CHECK_RESULT(
                        vkCreateDescriptorSetLayout(handle->get_device(), &descriptorLayout, nullptr, &
                            descriptorSetLayoutTex));
    }

    inline VkDescriptorSetLayout create_descriptor_set_layout(VKDevice &handle) {
        VkDescriptorSetLayout descriptorSetLayout;

        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
            descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0),
            descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1),
        };
        VkDescriptorSetLayoutCreateInfo descriptorLayout = descriptorSetLayoutCreateInfo(setLayoutBindings);
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(handle.get_device(), &descriptorLayout, nullptr, &
                            descriptorSetLayout));
        return descriptorSetLayout;
    }


    /**
     * 池、数量以及布局（Layout）
     * @param size
     */
    void AllocateDescriptorSets(uint32_t size) {
        uint32_t variableDescCount{size};
        // Vulkan 协议强制规定：只有索引号（Binding Number）最大的那一个绑定可以是可变的
        // 位置限制： 只有描述符集布局中 Binding 编号最大 的那个绑定才能设置为可变长度。
        // 上限约束： 你在 pDescriptorCounts 中指定的数值，不能超过你在 VkDescriptorSetLayoutBinding 中定义的 descriptorCount（即最大上限）。
        // 特性开启： 需要在物理设备特性中开启 descriptorIndexing 的相关支持，具体可参考 Vulkan 硬件数据库 检查你的显卡是否支持 runtimeDescriptorArray
        VkDescriptorSetVariableDescriptorCountAllocateInfo variableDescCountAI{
            .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT,
            .descriptorSetCount = 1,
            .pDescriptorCounts  = &variableDescCount
        };


        VkDescriptorSetAllocateInfo texDescSetAlloc{
            .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, .
            pNext               = &variableDescCountAI,
            .descriptorPool     = descriptor_pool_->get_pool(),
            .descriptorSetCount = 1,                      // // 打算分配的集合数量
            .pSetLayouts        = &descriptorSetLayoutTex // 指向布局数组的指针,长度必须等于 descriptorSetCount
        };
        VK_CHECK_RESULT(vkAllocateDescriptorSets(handle->get_device(), &texDescSetAlloc, &descriptor_set_texture));
    }


    void Destroy() {
        vkDestroyDescriptorSetLayout(handle->get_device(), descriptorSetLayoutTex, nullptr);
    }


    VkPipelineLayout CreatePipelineLayout() {
        VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
        VkPushConstantRange pushConstantRange{
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, .size = sizeof(VkDeviceAddress)
        };
        VkPipelineLayoutCreateInfo pipelineLayoutCI{
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount         = 1,
            .pSetLayouts            = &descriptorSetLayoutTex,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges    = &pushConstantRange
        };
        VK_CHECK_RESULT(vkCreatePipelineLayout(handle->get_device(), &pipelineLayoutCI, nullptr, &pipelineLayout));
        return pipelineLayout;
    }


    void update_descriptor_sets(std::vector<VkDescriptorImageInfo> textureDescriptors) {
        VkWriteDescriptorSet writeDescSet{
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet          = descriptor_set_texture,
            .dstBinding      = 0,
            .descriptorCount = static_cast<uint32_t>(textureDescriptors.size()),
            .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo      = textureDescriptors.data()
        };
        vkUpdateDescriptorSets(handle->get_device(), 1, &writeDescSet, 0, nullptr);
    }
};


#endif //HOWTOVULKAN_DESCRIPTOR_H
