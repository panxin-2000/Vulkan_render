//
// Created by 潘鑫 on 2026/1/26.
//

#ifndef HOWTOVULKAN_CREATE_PIPELINE_H
#define HOWTOVULKAN_CREATE_PIPELINE_H
#include <vector>
#include <volk.h>

#include "logic_render_data.h"
#include "vulkan_device_handle.h"

template<typename T>
uint32_t to_u32(T val) {
    assert(val <= std::numeric_limits<uint32_t>::max());
    return static_cast<uint32_t>(val);
}

inline VkPipelineVertexInputStateCreateInfo create_vertex_input_state() {
    VkVertexInputBindingDescription vertexBinding{
        .binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
    std::vector<VkVertexInputAttributeDescription> vertexAttributes{
        {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT},
        {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, normal)},
        {.location = 2, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, uv)},
    };
    VkPipelineVertexInputStateCreateInfo vertexInputState{
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount   = 1,
        .pVertexBindingDescriptions      = &vertexBinding,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributes.size()),
        .pVertexAttributeDescriptions    = vertexAttributes.data(),
    };
    return vertexInputState;
}

struct PipelineVertexInputState {
    std::shared_ptr<std::vector<VkVertexInputBindingDescription> > vertexBinding_copy;
    std::shared_ptr<std::vector<VkVertexInputAttributeDescription> > vertexAttributes_copy;
    std::shared_ptr<VkPipelineVertexInputStateCreateInfo> vertexInputState_copy;

    VkPipelineVertexInputStateCreateInfo *get_to_bind() const {
        return vertexInputState_copy.get();
    }
};

inline auto vertex_input_position_normal_uv() {
    std::vector<VkVertexInputBindingDescription> vertexBindings{
        {.binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX},
    };
    auto vertexBinding_copy = std::make_shared<decltype (vertexBindings)>(vertexBindings);
    std::vector<VkVertexInputAttributeDescription> vertexAttributes{
        {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT},
        {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, normal)},
        {.location = 2, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, uv)},
    };
    auto vertexAttributes_copy = std::make_shared<decltype (vertexAttributes)>(vertexAttributes);
    VkPipelineVertexInputStateCreateInfo vertexInputState{
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount   = static_cast<uint32_t>(vertexBinding_copy->size()),
        .pVertexBindingDescriptions      = vertexBinding_copy->data(), // 这里是引用
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributes_copy->size()),
        .pVertexAttributeDescriptions    = vertexAttributes_copy->data(), // 这里也是引用
    };
    auto vertexInputState_copy = std::make_shared<decltype (vertexInputState)>(vertexInputState);


    PipelineVertexInputState return_struct{vertexBinding_copy, vertexAttributes_copy, vertexInputState_copy};
    return return_struct;
}

inline auto vertex_input_position_uv() {
    struct Vertex {
        glm::vec3 pos;
        glm::vec2 uv;
    };
    std::vector<VkVertexInputBindingDescription> vertexBindings{
        {.binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX},
    };
    auto vertexBinding_copy = std::make_shared<decltype (vertexBindings)>(vertexBindings);
    std::vector<VkVertexInputAttributeDescription> vertexAttributes{
        {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT},
        {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, uv)},
    };
    auto vertexAttributes_copy = std::make_shared<decltype (vertexAttributes)>(vertexAttributes);
    VkPipelineVertexInputStateCreateInfo vertexInputState{
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount   = static_cast<uint32_t>(vertexBinding_copy->size()),
        .pVertexBindingDescriptions      = vertexBinding_copy->data(), // 这里是引用
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributes_copy->size()),
        .pVertexAttributeDescriptions    = vertexAttributes_copy->data(), // 这里也是引用
    };
    auto vertexInputState_copy = std::make_shared<decltype (vertexInputState)>(vertexInputState);


    PipelineVertexInputState return_struct{vertexBinding_copy, vertexAttributes_copy, vertexInputState_copy};
    return return_struct;
}

inline auto vertex_input_position() {
    struct Vertex {
        glm::vec3 pos;
    };
    std::vector<VkVertexInputBindingDescription> vertexBindings{
        {.binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX},
    };
    auto vertexBinding_copy = std::make_shared<decltype (vertexBindings)>(vertexBindings);
    std::vector<VkVertexInputAttributeDescription> vertexAttributes{
        {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT},
    };
    auto vertexAttributes_copy = std::make_shared<decltype (vertexAttributes)>(vertexAttributes);
    VkPipelineVertexInputStateCreateInfo vertexInputState{
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount   = static_cast<uint32_t>(vertexBinding_copy->size()),
        .pVertexBindingDescriptions      = vertexBinding_copy->data(), // 这里是引用
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributes_copy->size()),
        .pVertexAttributeDescriptions    = vertexAttributes_copy->data(), // 这里也是引用
    };
    auto vertexInputState_copy = std::make_shared<decltype (vertexInputState)>(vertexInputState);


    PipelineVertexInputState return_struct{vertexBinding_copy, vertexAttributes_copy, vertexInputState_copy};
    return return_struct;
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

inline VkPipeline create_graphics_pipeline(VK_handle &handle,
                                           std::vector<VkPipelineShaderStageCreateInfo> &shaderStages,
                                           VkPipelineLayout pipelineLayout,
                                           VkPipelineVertexInputStateCreateInfo *vertexInputState) {
    // Pipeline
    VkPipeline pipeline{VK_NULL_HANDLE};

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{
        .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    };
    std::vector<VkDynamicState> dynamicStates{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{
        .sType          = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, .dynamicStateCount = 2,
        .pDynamicStates = dynamicStates.data()
    };
    VkPipelineViewportStateCreateInfo viewportState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .viewportCount = 1, .scissorCount = 1
    };
    VkPipelineRasterizationStateCreateInfo rasterizationState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, .lineWidth = 1.0f
    };
    VkPipelineMultisampleStateCreateInfo multisampleState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };
    VkPipelineDepthStencilStateCreateInfo depthStencilState{
        .sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE, .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL
    };
    VkPipelineColorBlendAttachmentState blendAttachment{.colorWriteMask = 0xF};
    VkPipelineColorBlendStateCreateInfo colorBlendState{
        .sType        = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, .attachmentCount = 1,
        .pAttachments = &blendAttachment
    };
    VkFormat pColorAttachmentFormats = handle.get_image_format();
    VkPipelineRenderingCreateInfo renderingCI{
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO, .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &pColorAttachmentFormats, .depthAttachmentFormat = handle.get_depth_format()
    };
    VkGraphicsPipelineCreateInfo pipelineCI{
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = &renderingCI,
        .stageCount          = to_u32(shaderStages.size()),
        .pStages             = shaderStages.data(),
        .pVertexInputState   = vertexInputState,
        .pInputAssemblyState = &inputAssemblyState,
        .pViewportState      = &viewportState,
        .pRasterizationState = &rasterizationState,
        .pMultisampleState   = &multisampleState,
        .pDepthStencilState  = &depthStencilState,
        .pColorBlendState    = &colorBlendState,
        .pDynamicState       = &dynamicState,
        .layout              = pipelineLayout
    };
    VK_CHECK_RESULT_NOT_EXIT(vkCreateGraphicsPipelines(handle.get_device(), VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &
                                 pipeline));
    return pipeline;
}


inline VkPipeline create_pipeline(VK_handle &handle, const std::string &shader_key,
                                  VkPipelineLayout pipelineLayout,
                                  std::vector<VkPipelineShaderStageCreateInfo> &shaderStages,
                                  std::map<std::string, pipeline_and_share> &map) {
    if (!shader_key.empty()) {
        auto it = map.find(shader_key);
        if (it != map.end()) {
            it->second.shared_number++;
            return it->second.pipeline;
        } else {
            const auto vertexInputState = vertex_input_position_normal_uv();
            auto pipeline               = create_graphics_pipeline(handle, shaderStages, pipelineLayout,
                                                                   vertexInputState.get_to_bind());
            map.insert({shader_key, {pipeline, 1}});
            return pipeline;
        }
    }
    return VK_NULL_HANDLE;
}

inline VkPipeline find_pipeline(VK_handle &handle, const std::string &shader_key,
                                VkPipelineLayout pipelineLayout,
                                std::vector<VkPipelineShaderStageCreateInfo> &shaderStages,
                                std::map<std::string, pipeline_and_share> &map) {
    if (!shader_key.empty()) {
        auto it = map.find(shader_key);
        if (it != map.end()) {
            return it->second.pipeline;
        } else {
            return create_pipeline(handle, shader_key, pipelineLayout, shaderStages, map);
        }
    }
    return VK_NULL_HANDLE;
}


inline void clean_all_pipeline(VK_handle &handle) {
    auto pipeline_map = VK_handle::get().get_pipeline_map();
    for (const auto &[key, value]: pipeline_map) {
        vkDestroyPipeline(handle.get_device(), value.pipeline, nullptr);
    }
    pipeline_map.clear();
}
#endif //HOWTOVULKAN_CREATE_PIPELINE_H
