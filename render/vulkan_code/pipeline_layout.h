//
// Created by 潘鑫 on 2026/2/9.
//

#ifndef HELLO_MAC_PIPELINE_LAYOUT_H
#define HELLO_MAC_PIPELINE_LAYOUT_H

#include "vulkan_backend.h"


/**
 * pipeline_layout 只是一个接口
 * 创建管线描述符布局, set = 0 还是 set = 1 需要在这里设置，解析需要更靠前
 * organize_graphics_descriptor_set_layouts  是 organize_graphics_descriptor_set_layouts 作为参数
 * 经由 create_descriptor_set_layouts 得出的结果
 * @param handle
 * @param shader_key
 * @param descriptor_sets_layout  layout(set = 0, binding = 0) layout(set = 1, binding = 0)
 * @param push_constant_map
 * @return
 */
VkPipelineLayout create_pipeline_layout(VK_backend &handle, const std::string shader_key,
                                        std::vector<VkDescriptorSetLayout> descriptor_sets_layout,
                                        std::map<std::string, VkPushConstantRange> &push_constant_map);





#endif //HELLO_MAC_PIPELINE_LAYOUT_H
