//
// Created by 潘鑫 on 2026/3/27.
//

#ifndef HELLO_MAC_PIPELINE_LAYOUT_COMPONENT_H
#define HELLO_MAC_PIPELINE_LAYOUT_COMPONENT_H
#include "global_singleton.h"
#include "shader_component.h"
#include "vulkan_backend.h"

inline VkPipelineLayout get_pipeline_layout(const entt::entity entity) {
    auto &handle = VK_backend::instance();
    if (const auto &shader_data_ref = Render_entt().get<Shader_data>(entity)) {
        const VkPipelineLayout pipeline_layout = shader_data_ref->pipeline_layout;
        return pipeline_layout;
    } else {
        // 打印一个 entity name 没有 VKR_shader
    }
    return VK_NULL_HANDLE;
}
#endif //HELLO_MAC_PIPELINE_LAYOUT_COMPONENT_H
