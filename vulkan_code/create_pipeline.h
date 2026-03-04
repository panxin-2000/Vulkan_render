//
// Created by 潘鑫 on 2026/1/26.
//

#ifndef HOWTOVULKAN_CREATE_PIPELINE_H
#define HOWTOVULKAN_CREATE_PIPELINE_H
#include <vector>
#include <volk.h>

#include "Geometry_data.h"
#include "vulkan_device_handle.h"

template<typename T>
uint32_t to_u32(T val) {
    assert(val <= std::numeric_limits<uint32_t>::max());
    return static_cast<uint32_t>(val);
}


/**
 * 目前在 binding = 0 的情况下还没有出错过，其他的尽量用 SSBO 来保存与更改
 * @param vertexBindings
 * @param vertexAttributes
 */
inline auto VertexInputStateFunction(std::vector<VkVertexInputBindingDescription> &vertexBindings,
                                     std::vector<VkVertexInputAttributeDescription> &vertexAttributes) {
    const VkPipelineVertexInputStateCreateInfo vertexInputState{
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount   = static_cast<uint32_t>(vertexBindings.size()),
        .pVertexBindingDescriptions      = vertexBindings.data(),
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributes.size()),
        .pVertexAttributeDescriptions    = vertexAttributes.data(),
    };
    return vertexInputState;
}


inline VkPipeline CreateComputePipelines(VK_handle &handle, std::vector<VkPipelineShaderStageCreateInfo> &shaderStages,
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

inline VkPipeline create_graphics_pipeline(VK_handle &handle, vk_shader_data &data) {
    // Pipeline
    VkPipeline pipeline{VK_NULL_HANDLE};
    std::vector<VkPipelineShaderStageCreateInfo> &shaderStages       = data.pipeline_shader_stage_create_infos;
    VkPipelineLayout &pipelineLayout                                 = data.pipeline_layout;
    std::vector<VkVertexInputBindingDescription> &vertexBindings     = data.vertexBindings;
    std::vector<VkVertexInputAttributeDescription> &vertexAttributes = data.vertexAttributes;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{
        .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    };
    std::vector<VkDynamicState> dynamicStates{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates    = dynamicStates.data()
    };
    VkPipelineViewportStateCreateInfo viewportState{
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount  = 1
    };
    VkPipelineRasterizationStateCreateInfo rasterizationState{
        .sType     = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .lineWidth = 1.0f
    };
    VkPipelineMultisampleStateCreateInfo multisampleState{
        .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };
    VkPipelineDepthStencilStateCreateInfo depthStencilState{
        .sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable  = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp   = VK_COMPARE_OP_LESS_OR_EQUAL
    };
    VkPipelineColorBlendAttachmentState blendAttachment{.colorWriteMask = 0xF};
    VkPipelineColorBlendStateCreateInfo colorBlendState{
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments    = &blendAttachment
    };
    VkFormat pColorAttachmentFormats = handle.get_image_format();
    VkPipelineRenderingCreateInfo renderingCI{
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount    = 1,
        .pColorAttachmentFormats = &pColorAttachmentFormats,
        .depthAttachmentFormat   = handle.get_depth_format()
    };
    auto vertexInputState = VertexInputStateFunction(vertexBindings, vertexAttributes);
    VkGraphicsPipelineCreateInfo pipelineCI{
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = &renderingCI,
        .stageCount          = to_u32(shaderStages.size()), //这个是由shader决定的
        .pStages             = shaderStages.data(),
        .pVertexInputState   = &vertexInputState,
        .pInputAssemblyState = &inputAssemblyState,
        .pViewportState      = &viewportState,
        .pRasterizationState = &rasterizationState,
        .pMultisampleState   = &multisampleState,
        .pDepthStencilState  = &depthStencilState,
        .pColorBlendState    = &colorBlendState,
        .pDynamicState       = &dynamicState,
        .layout              = pipelineLayout //这个是由shader决定的
    };
    VK_CHECK_RESULT_NOT_EXIT(vkCreateGraphicsPipelines(handle.get_device(), VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &
                                 pipeline));
    return pipeline;
}


VkPipeline create_pipeline(VK_handle &handle, vk_shader_data &data);

VkPipeline find_pipeline(VK_handle &handle, vk_shader_data &data);


void clean_all_pipeline(VK_handle &handle);
#endif //HOWTOVULKAN_CREATE_PIPELINE_H
