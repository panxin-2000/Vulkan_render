//
// Created by 潘鑫 on 2026/3/4.
//
#include "create_pipeline.h"

#include "shader_common.h"


/**
 * 目前在 binding = 0 的情况下还没有出错过，其他的尽量用 SSBO 来保存与更改
 * @param vertexBindings
 * @param vertexAttributes
 */
inline auto VertexInputStateFunction(std::vector<VkVertexInputBindingDescription> &vertexBindings,
                                     std::vector<VkVertexInputAttributeDescription> &vertexAttributes) {
    const VkPipelineVertexInputStateCreateInfo vertexInputState{
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount   = to_u32(vertexBindings.size()),
        .pVertexBindingDescriptions      = vertexBindings.data(),
        .vertexAttributeDescriptionCount = to_u32(vertexAttributes.size()),
        .pVertexAttributeDescriptions    = vertexAttributes.data(),
    };
    return vertexInputState;
}


VkPipeline create_compute_pipeline(VK_backend &backend, vk_shader_data &data) {
    VkPipeline pipeline = VK_NULL_HANDLE;

    VkComputePipelineCreateInfo pipelineCreateInfo = {VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
    pipelineCreateInfo.sType                       = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stage                       = data.computer_shader_stage_create_infos.at(0);
    pipelineCreateInfo.layout                      = data.pipeline_layout;
    pipelineCreateInfo.flags                       = 0;
    pipelineCreateInfo.basePipelineHandle          = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex           = 0;
    pipelineCreateInfo.pNext                       = nullptr;

    VK_CHECK_RESULT(vkCreateComputePipelines(backend.get_device(),
                        nullptr,
                        1,
                        &pipelineCreateInfo,
                        nullptr,
                        &pipeline));
    return pipeline;
}

VkPipeline create_compute_or_graphics_pipeline(VK_backend &backend, vk_shader_data &data) {
    if (!data.pipeline_shader_stage_create_infos.empty()) {
        return create_graphics_pipeline(backend, data);
    }
    if (!data.computer_shader_stage_create_infos.empty()) {
        return create_compute_pipeline(backend, data);
    }
    return VK_NULL_HANDLE;
}


VkPipeline create_graphics_pipeline(VK_backend &backend, vk_shader_data &data) {
    // Pipeline
    VkPipeline pipeline{VK_NULL_HANDLE};
    std::vector<VkPipelineShaderStageCreateInfo> &shaderStages      = data.pipeline_shader_stage_create_infos;
    VkPipelineLayout &pipelineLayout                                = data.pipeline_layout;
    std::vector<VkVertexInputBindingDescription> &vertexBindings    = data.vertexBindings;
    std::vector<VkVertexInputAttributeDescription> vertexAttributes = data.get_vertexAttributes();
    Fragment_output_map &colorAttachmentFormat                      = data.fragment_output_map;

    auto vertexInputState = VertexInputStateFunction(vertexBindings, vertexAttributes);


    /******************************** 动态状态 **********************************/
    std::vector<VkDynamicState> dynamicStates{
        VK_DYNAMIC_STATE_VIEWPORT, // vkCmdSetViewport
        VK_DYNAMIC_STATE_SCISSOR,  // vkCmdSetScissor

        // VkPipelineDepthStencilStateCreateInfo
        VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,  // vkCmdSetDepthTestEnable
        VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE, // vkCmdSetDepthWriteEnable
        VK_DYNAMIC_STATE_DEPTH_COMPARE_OP,   // vkCmdSetDepthCompareOp

        VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE, // vkCmdSetDepthBoundsTestEnable
        VK_DYNAMIC_STATE_DEPTH_BOUNDS,             // vkCmdSetDepthBounds

        VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE, // vkCmdSetStencilTestEnable
        VK_DYNAMIC_STATE_STENCIL_OP,          // vkCmdSetStencilOp          vkCmdSetStencilOp

        VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
        VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
        VK_DYNAMIC_STATE_STENCIL_REFERENCE,


        // VkPipelineRasterizationStateCreateInfo
        VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE, // vkCmdSetDepthBiasEnable
        VK_DYNAMIC_STATE_DEPTH_BIAS,        // vkCmdSetDepthBias
        VK_DYNAMIC_STATE_CULL_MODE,         // vkCmdSetFrontFace
        VK_DYNAMIC_STATE_FRONT_FACE,        // vkCmdSetCullMode
    };
    VkPipelineDynamicStateCreateInfo dynamicState{
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates    = dynamicStates.data()
    };
    VkPipelineViewportStateCreateInfo viewportState{
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount  = 1
    };
    VkPipelineRasterizationStateCreateInfo rasterizationState{
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = 0,
        .depthClampEnable        = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode             = VK_POLYGON_MODE_FILL, // todo: 需要注意
        .cullMode                = VK_CULL_MODE_NONE,
        .frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable         = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp          = 0.0f,
        .depthBiasSlopeFactor    = 0.0f,
        .lineWidth               = 1.0f, // 在 VK_POLYGON_MODE_LINE 模式下，线的粗细由该结构体中的 lineWidth 成员控制
    };
    VkPipelineDepthStencilStateCreateInfo depthStencilState{
        .sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable  = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp   = VK_COMPARE_OP_LESS_OR_EQUAL
    };
    /******************************** 动态状态 **********************************/


    /******************************** 下面的需要需要通过参数更改 **********************************/
    VkPipelineMultisampleStateCreateInfo multisampleState{
        .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };

    // 这里有问题，但是需要着色器上的一些内容
    std::vector<VkPipelineColorBlendAttachmentState> BlendAttachments{};
    BlendAttachments.resize(colorAttachmentFormat.size());
    for (size_t i = 0; i < colorAttachmentFormat.size(); ++i) {
        // 这里的参数很多，没有写入值
        VkPipelineColorBlendAttachmentState blendAttachment{
            .blendEnable         = VK_TRUE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp        = VK_BLEND_OP_ADD,
            .colorWriteMask      = 0xF,
        };
        BlendAttachments[i] = blendAttachment;
    }
    VkPipelineColorBlendStateCreateInfo colorBlendState{
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(BlendAttachments.size()),
        .pAttachments    = BlendAttachments.data()
    };
    std::vector<VkFormat> pColorAttachmentFormats{};
    for (const auto &[fst, snd]: colorAttachmentFormat) {
        pColorAttachmentFormats.push_back(snd.format);
    }

    VkPipelineRenderingCreateInfo renderingCI{
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount    = static_cast<uint32_t>(pColorAttachmentFormats.size()),
        .pColorAttachmentFormats = pColorAttachmentFormats.data(),
        .depthAttachmentFormat   = backend.get_depth_format(),
        .stencilAttachmentFormat = backend.get_depth_format()
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{
        .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = data.topology
    };
    VkGraphicsPipelineCreateInfo pipelineCI{
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = &renderingCI,
        .stageCount          = to_u32(shaderStages.size()), //不能动态
        .pStages             = shaderStages.data(),         // 不能动态
        .pVertexInputState   = &vertexInputState,           // 不能动态
        .pInputAssemblyState = &inputAssemblyState,         // 不建议动态
        .pViewportState      = &viewportState,              // 建议动态
        .pRasterizationState = &rasterizationState,         // 有三个参数建议
        .pMultisampleState   = &multisampleState,           // 不建议动态
        .pDepthStencilState  = &depthStencilState,          // 建议动态
        .pColorBlendState    = &colorBlendState,            // 暂时不设置
        .pDynamicState       = &dynamicState,               // 设置动态相关内容
        .layout              = pipelineLayout               // 不能动态
    };
    VK_CHECK_RESULT_NOT_EXIT(vkCreateGraphicsPipelines(backend.get_device(),
                                 VK_NULL_HANDLE,
                                 1,
                                 &pipelineCI,
                                 nullptr,
                                 &pipeline));
    return pipeline;
}


VkPipeline create_pipeline(VK_backend &backend, vk_shader_data &data) {
    auto pipeline = create_compute_or_graphics_pipeline(backend, data);
    return pipeline;
}


VkPipeline CreateComputePipelines(VK_backend &handle, std::vector<VkPipelineShaderStageCreateInfo> &shaderStages,
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
