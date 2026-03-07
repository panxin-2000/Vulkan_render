//
// Created by 潘鑫 on 2026/3/3.
//

#ifndef HELLO_MAC_RENDER_PROXY_H
#define HELLO_MAC_RENDER_PROXY_H
#include "descriptor.h"
#include "mesh_component.h"


class VKR_object_proxy {
public:
    std::string debug_name;
    VkPipeline vk_pipeline;
    VkPipelineLayout pipeline_layout;
    std::vector<DescriptorSet_ptr> vk_descriptor_set; // descriptor_set 的 共享指针保存点
    VkViewport viewport;
    VkRect2D scissor;
    VKR_buffer_block_ptr push_constants_address;
    std::optional<float> line_width;
    VKR_mesh mesh;
};

#endif //HELLO_MAC_RENDER_PROXY_H
