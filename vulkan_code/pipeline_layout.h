//
// Created by 潘鑫 on 2026/2/9.
//

#ifndef HELLO_MAC_PIPELINE_LAYOUT_H
#define HELLO_MAC_PIPELINE_LAYOUT_H

#include "vulkan_device_handle.h"

/**
 * pipeline_layout 只是一个接口
 * 创建管线描述符布局, set = 0 还是 set = 1 需要在这里设置，解析需要更靠前
 * organize_graphics_descriptor_set_layouts  是 organize_graphics_descriptor_set_layouts 作为参数
 * 经由 create_descriptor_set_layouts 得出的结果
 * @param handle
 * @param descriptor_sets_layout  layout(set = 0, binding = 0) layout(set = 1, binding = 0)
 * @return
 */
inline VkPipelineLayout create_pipeline_layout(const VKDevice &handle,
                                               std::vector<VkDescriptorSetLayout> descriptor_sets_layout) {
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    VkPushConstantRange pushConstantRange{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, .size = sizeof(VkDeviceAddress)
    };
    VkPipelineLayoutCreateInfo pipelineLayoutCI{
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = static_cast<uint32_t>(descriptor_sets_layout.size()),
        .pSetLayouts            = descriptor_sets_layout.data(),
        .pushConstantRangeCount = 1,
        .pPushConstantRanges    = &pushConstantRange
    };
    // VkPipelineLayout 的本质是 “接口协议”（Interface Protocol）。
    // 它定义了 Shader 如何访问资源（比如有哪些 Set，每个 Set 有哪些 Binding）。
    VK_CHECK_RESULT_NOT_EXIT(vkCreatePipelineLayout(handle.get_device(), &pipelineLayoutCI, nullptr, &pipeline_layout));
    return pipeline_layout;
}


#endif //HELLO_MAC_PIPELINE_LAYOUT_H
