//
// Created by 潘鑫 on 2026/3/27.
//

#ifndef HELLO_MAC_PIPELINE_LAYOUT_COMPONENT_H
#define HELLO_MAC_PIPELINE_LAYOUT_COMPONENT_H
#include "global_singleton.h"
#include "shader_component.h"
#include "vulkan_backend.h"

inline VkPipelineLayout get_pipeline_layout(const entt::entity entity) {
    if (const auto &shader_data_ref = Render_entt().get<Shader_data>(entity)) {
        const VkPipelineLayout pipeline_layout = shader_data_ref->pipeline_layout;
        return pipeline_layout;
    }
    return VK_NULL_HANDLE;
}
#endif //HELLO_MAC_PIPELINE_LAYOUT_COMPONENT_H
