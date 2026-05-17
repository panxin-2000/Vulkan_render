//
// Created by 潘鑫 on 2026/1/26.
//

#ifndef HOWTOVULKAN_CREATE_PIPELINE_H
#define HOWTOVULKAN_CREATE_PIPELINE_H
#include <vector>
#include <volk.h>

#include "shader_component.h"
#include "vulkan_backend.h"


/**
 * 没有具体的写明白这个函数
 * @param handle
 * @param shaderStages
 * @param descriptorSetLayout
 * @return
 */
inline VkPipeline CreateComputePipelines(VK_backend &handle, std::vector<VkPipelineShaderStageCreateInfo> &shaderStages,
                                         VkDescriptorSetLayout &descriptorSetLayout) {
    if (shaderStages.empty() == true) {
        return VK_NULL_HANDLE;
    }


    VkPipelineLayout pipelineLayout;
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts    = &descriptorSetLayout;

    VK_CHECK_RESULT_NOT_EXIT(vkCreatePipelineLayout(handle.get_device(), &pipelineLayoutCreateInfo, nullptr, &
                                 pipelineLayout));


    VkPipeline compute_pipeline = VK_NULL_HANDLE;
    VkComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.layout = pipelineLayout;
    computePipelineCreateInfo.flags  = 0;
    computePipelineCreateInfo.stage  = shaderStages[0];


    VK_CHECK_RESULT_NOT_EXIT(vkCreateComputePipelines(handle.get_device(),
                                 VK_NULL_HANDLE,
                                 1, &computePipelineCreateInfo,
                                 nullptr,
                                 &compute_pipeline));
    return compute_pipeline;
}

VkPipeline create_graphics_pipeline(VK_backend &backend, vk_shader_data &data);

VkPipeline create_pipeline(VK_backend &backend, vk_shader_data &data);

VkPipeline find_pipeline(VK_backend &handle, std::shared_ptr<vk_shader_data> &data);


void clean_all_pipeline(VK_backend &handle);
#endif //HOWTOVULKAN_CREATE_PIPELINE_H
