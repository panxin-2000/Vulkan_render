//
// Created by 潘鑫 on 2026/3/3.
//

#ifndef HELLO_MAC_RENDER_PROXY_H
#define HELLO_MAC_RENDER_PROXY_H
#include "descriptor.h"
#include "mesh_component.h"


class VKR_object_proxy {
public:
    std::string pass_name;
    std::string debug_name;
    VkPipeline vk_pipeline;
    VkPipelineLayout pipeline_layout;
    std::vector<DescriptorSet_ptr> vk_descriptor_set; // descriptor_set 的 共享指针保存点
    VkViewport viewport;
    VkRect2D scissor;
    std::byte push_constants_pool[128];
    std::optional<float> line_width;
    VKR_mesh mesh;
};


class Render_entity {
public:
    entt::entity entity_;
};


struct deferred_pass_tag {
};


struct RND_destroy_tag {
};

#endif //HELLO_MAC_RENDER_PROXY_H
