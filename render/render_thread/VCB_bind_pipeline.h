//
// Created by 潘鑫 on 2026/8/13.
//

#ifndef HELLO_MAC_VULKAN_BUILD_BIND_PIPELINE_H
#define HELLO_MAC_VULKAN_BUILD_BIND_PIPELINE_H

#include "GPU_frustum_cull.h"
#include "../vulkan_code/descriptor.h"
#include "../engine.h"
#include "render_proxy.h"
#include "../vulkan_code/vertex_and_buffer_index.h"
#include "name_component.h"
#include "../render_common/render_state.h"
#include "../render_common/render_mesh.h"
#include "vulkan_update_descriptor.h"

inline void bind_Proxy_descriptor_sets(VK_backend &engine, entt::entity entity, const uint64_t time_line,
                                       VkPipelineBindPoint bind_point) {
    const auto cb                 = Engine::instance().get_current_command_buffer();
    const auto vk_descriptor_sets = update_descriptor_sets(entity);
    const auto &shader_data_ref   = Render_entt().get<shader_data>(entity);

    if (!vk_descriptor_sets.empty()) {
        std::vector<VkDescriptorSet> temp_descriptor_sets;
        temp_descriptor_sets.resize(vk_descriptor_sets.size());
        for (size_t i = 0; i < vk_descriptor_sets.size(); ++i) {
            temp_descriptor_sets[i] = vk_descriptor_sets[i]->get_descriptor_set(time_line);
            // LOG_INFO(g_log(), "temp_descriptor_sets[{}] = {}", i, (uint64_t)temp_descriptor_sets[i]);
        }
        std::vector<uint32_t> dynamic_offsets; // dynamic
        // dynamic_offsets.resize(vk_descriptor_sets.size());
        // for (size_t i = 0; i < vk_descriptor_sets.size(); ++i) {
        // dynamic_offsets[i] = 0;
        // }
        for (auto temp_descriptor_set: temp_descriptor_sets) {
            if (temp_descriptor_set == VK_NULL_HANDLE) {
                LOG_INFO(g_log(), "VKR_object_proxy {} descriptor_set == VK_NULL_HANDLE ",
                         Render_entt().get<Name_component>(entity).name_);
                return;
            }
        }
        vkCmdBindDescriptorSets(cb, bind_point,
                                shader_data_ref->pipeline_layout,
                                0,
                                temp_descriptor_sets.size(),
                                temp_descriptor_sets.data(),
                                dynamic_offsets.size(),
                                dynamic_offsets.data());
    }
}


inline void bind_pipeline_update_parameter(VK_backend &engine, entt::entity entity, const uint64_t time_line) {
    const auto cb = Engine::instance().get_current_command_buffer();

    auto debug_name             = Render_entt().get<Name_component>(entity).name_;
    const auto &shader_data_ref = Render_entt().get<shader_data>(entity);
    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, shader_data_ref->pipeline_t);


    bind_Proxy_descriptor_sets(engine, entity, time_line, VK_PIPELINE_BIND_POINT_GRAPHICS);

    // VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT 允许不绑定部分描述符，只要不犯法就是允许的
    // 访问的时候不在也是可以的，不会出现明显的死机，只是内容没有绘制

    //  shader_data 还没有传送过来
    if (const auto parameter = Render_entt().try_get<shader_constant_parameter>(entity))
        for (auto &[name,value]: shader_data_ref->push_constant_map) {
            // 我的建议是 每次 直接全部复制 128 字节，哪怕全部都是空的占位符 也是 如此
            vkCmdPushConstants(cb, shader_data_ref->pipeline_layout,
                               value.stageFlags, value.offset, value.size,
                               parameter->push_constant_pool + value.offset);
        }
}


#endif //HELLO_MAC_VULKAN_BUILD_BIND_PIPELINE_H
