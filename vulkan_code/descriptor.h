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
     * @param size   static_cast<uint32_t>(textures.size())
     */
    void CreateDescriptorSetLayout(uint32_t size) {
        // Descriptor (indexing)
        VkDescriptorBindingFlags descVariableFlag{VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT};
        VkDescriptorSetLayoutBindingFlagsCreateInfo descBindingFlags{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO, .bindingCount = 1,
            .pBindingFlags = &descVariableFlag
        };
        VkDescriptorSetLayoutBinding descLayoutBindingTex{
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = size,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
        };
        VkDescriptorSetLayoutCreateInfo descLayoutTexCI{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &descBindingFlags,
            .bindingCount = 1,
            .pBindings = &descLayoutBindingTex
        };
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(handle->get_device(), &descLayoutTexCI, nullptr, &descriptorSetLayoutTex));
    }

    /**
     * 池、数量以及布局（Layout）
     * @param size
     */
    void AllocateDescriptorSets(uint32_t size) {
        uint32_t variableDescCount{size};
        VkDescriptorSetVariableDescriptorCountAllocateInfo variableDescCountAI{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT,
            .descriptorSetCount = 1,
            .pDescriptorCounts = &variableDescCount
        };


        VkDescriptorSetAllocateInfo texDescSetAlloc{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, .
            pNext = &variableDescCountAI,
            .descriptorPool = descriptor_pool_->get_pool(),
            .descriptorSetCount = 1,       // // 打算分配的集合数量
            .pSetLayouts = &descriptorSetLayoutTex     // 指向布局数组的指针,长度必须等于 descriptorSetCount
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
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts = &descriptorSetLayoutTex,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &pushConstantRange
        };
        VK_CHECK_RESULT(vkCreatePipelineLayout(handle->get_device(), &pipelineLayoutCI, nullptr, &pipelineLayout));
        return pipelineLayout;
    }


    void update_descriptor_sets(std::vector<VkDescriptorImageInfo> textureDescriptors) {
        VkWriteDescriptorSet writeDescSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptor_set_texture,
            .dstBinding = 0,
            .descriptorCount = static_cast<uint32_t>(textureDescriptors.size()),
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = textureDescriptors.data()
        };
        vkUpdateDescriptorSets(handle->get_device(), 1, &writeDescSet, 0, nullptr);
    }
};


#endif //HOWTOVULKAN_DESCRIPTOR_H
