//
// Created by 潘鑫 on 2026/3/27.
//

#ifndef HELLO_MAC_PIPELINE_LAYOUT_H
#define HELLO_MAC_PIPELINE_LAYOUT_H
#include "global_singleton.h"
#include "shader_component.h"
#include "vulkan_backend.h"

inline VkPipelineLayout get_pipeline_layout(const entt::entity entity) {
    auto &handle                     = VK_backend::get();
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    if (const auto shader_temp = Logic_entt().try_get<VKR_shader_paths>(entity)) {
        if (!Logic_entt().all_of<std::shared_ptr<vk_shader_data> >(entity)) {
            Logic_entt().emplace<std::shared_ptr<vk_shader_data> >(entity, VKR_shader_init(*shader_temp));
        }
    }
    if (auto shader_data = Logic_entt().try_get<std::shared_ptr<vk_shader_data> >(entity)) {
        pipeline_layout = (*shader_data)->pipeline_layout;
        return pipeline_layout;
    } else {
        // 打印一个 entity name 没有 VKR_shader
    }
    return VK_NULL_HANDLE;
}
#endif //HELLO_MAC_PIPELINE_LAYOUT_H
