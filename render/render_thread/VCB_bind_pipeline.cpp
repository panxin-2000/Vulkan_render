//
// Created by 潘鑫 on 2026/8/13.
//


#include "../vulkan_code/descriptor.h"
#include "engine.h"
#include "name_component.h"
#include "VCB_vulkan_command_buffer.h"

#include "shader_component.h"

void VCB::bind_Proxy_descriptor_sets(entt::entity entity,
                                     VkPipelineLayout pipeline_layout,
                                     VkPipelineBindPoint bind_point) {
    const auto vk_descriptor_sets = update_descriptor_sets(entity);

    if (!vk_descriptor_sets.empty()) {
        std::vector<VkDescriptorSet> temp_descriptor_sets;
        temp_descriptor_sets.resize(vk_descriptor_sets.size());
        for (size_t i = 0; i < vk_descriptor_sets.size(); ++i) {
            temp_descriptor_sets[i] = vk_descriptor_sets[i]->get_descriptor_set(time_line_);
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
        vkCmdBindDescriptorSets(command_buffer_, bind_point,
                                pipeline_layout,
                                0,
                                temp_descriptor_sets.size(),
                                temp_descriptor_sets.data(),
                                dynamic_offsets.size(),
                                dynamic_offsets.data());
    }
}


void VCB::bind_pipeline_update_parameter(entt::entity entity, const Shader_data &shader_data_ref) {
    auto debug_name             = Render_entt().get<Name_component>(entity).name_;
    vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, shader_data_ref->pipeline_t);


    bind_Proxy_descriptor_sets(entity, shader_data_ref->pipeline_layout, VK_PIPELINE_BIND_POINT_GRAPHICS);

    // VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT 允许不绑定部分描述符，只要不犯法就是允许的
    // 访问的时候不在也是可以的，不会出现明显的死机，只是内容没有绘制

    //  shader_data 还没有传送过来
    if (const auto parameter = Render_entt().try_get<shader_constant_parameter>(entity))
        for (auto &[name,value]: shader_data_ref->push_constant_map) {
            // 我的建议是 每次 直接全部复制 128 字节，哪怕全部都是空的占位符 也是 如此
            vkCmdPushConstants(command_buffer_, shader_data_ref->pipeline_layout,
                               value.stageFlags, value.offset, value.size,
                               parameter->push_constant_pool + value.offset);
        }
}
