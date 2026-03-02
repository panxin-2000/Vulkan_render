//
// Created by 潘鑫 on 2026/3/3.
//

#ifndef HELLO_MAC_RENDER_PROXY_H
#define HELLO_MAC_RENDER_PROXY_H
#include "mesh_component.h"


class draw_need_vk {
public:
    std::string debug_name;
    VkPipeline vk_pipeline;
    VkPipelineLayout pipeline_layout;
    std::vector<VkDescriptorSet> vk_descriptor_set;
    VkViewport viewport;
    VkRect2D scissor;
    VkDeviceAddress push_constants_address;
    std::optional<float> line_width;
    Model_mesh mesh;
};

#endif //HELLO_MAC_RENDER_PROXY_H
