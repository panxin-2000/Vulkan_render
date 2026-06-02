//
// Created by 潘鑫 on 2026/3/27.
//

#ifndef HELLO_MAC_PIPELINE_COMPONENT_H
#define HELLO_MAC_PIPELINE_COMPONENT_H


#include "create_pipeline.h"


inline VkPipeline get_pipeline(const entt::entity entity) {
    auto &handle = VK_backend::instance();
    if (auto &shader_data_ref = Render_entt().get<shader_data>(entity)) {
        const VkPipeline pipeline_t = find_pipeline(handle, shader_data_ref);
        return pipeline_t;
    } else {
        // 打印一个 entity name 没有 VKR_shader
    }
    return VK_NULL_HANDLE;
}
#endif //HELLO_MAC_PIPELINE_COMPONENT_H
