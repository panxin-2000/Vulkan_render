//
// Created by 潘鑫 on 2026/3/27.
//

#ifndef HELLO_MAC_PIPELINE_COMPONENT_H
#define HELLO_MAC_PIPELINE_COMPONENT_H


#include "create_pipeline.h"


inline VkPipeline get_pipeline(const entt::entity entity) {
    auto &handle          = VK_backend::get();
    VkPipeline pipeline_t = VK_NULL_HANDLE;
    if (auto shader_data = Logic_entt().try_get<std::shared_ptr<vk_shader_data> >(entity)) {
        pipeline_t = find_pipeline(handle, *shader_data);
        return pipeline_t;
    } else {
        // 打印一个 entity name 没有 VKR_shader
    }
    return VK_NULL_HANDLE;
}
#endif //HELLO_MAC_PIPELINE_COMPONENT_H
