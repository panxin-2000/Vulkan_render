//
// Created by 潘鑫 on 2026/3/2.
//

#ifndef HELLO_MAC_SHADER_COMPONENT_H
#define HELLO_MAC_SHADER_COMPONENT_H
#include "create_pipeline.h"
#include "create_shader.h"
#include "descriptor.h"
#include "descriptor_organized_sets_and_bindings.h"
#include "Geometry_data.h"
#include "pipeline_layout.h"
#include "sets_and_bindings_layout.h"
#include "vertex_and_buffer_index.h"
#include "vk_render_to_image.h"
#include "vulkan_device_handle.h"
#include "vulkan_render_manage.h"


#include "update_push_constants_data.h"

#include <memory_resource>

void update_bindings_to_descriptor_sets(const entt::entity entity,
                                        const std::vector<VkDescriptorSet> &descriptor_sets);


template<typename T1>
bool add_uniform_buffer_data(const entt::entity entity, const std::string &binding_name, T1 binding_data) {
    if (const auto shader_temp = g_entt().try_get<VKR_shader>(entity)) {
        for (auto const &[set_value, bindings_map]:
             shader_temp->shader_data_handle->model_sets_bindings) {
            for (const auto &[binding_value, info]: bindings_map) {
                if (info.binding_name == binding_name && info.resource_type == "uniform buffer") {
                    auto buffer_block                   = copy_data_to_gpu_buffer(binding_data);
                    Update_descriptor_binding temp      = {};
                    temp.binding_name                   = binding_name;
                    temp.resource_type                  = info.resource_type;
                    temp.dstSet                         = set_value;
                    temp.descriptor_write_binding.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    // temp.descriptor_write_bindings.dstSet           = descriptor_sets[0];
                    temp.descriptor_write_binding.dstBinding       = 0;
                    temp.descriptor_write_binding.dstArrayElement  = 0;
                    temp.descriptor_write_binding.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    temp.descriptor_write_binding.descriptorCount  = 1;
                    temp.descriptor_write_binding.pBufferInfo      = nullptr;
                    temp.descriptor_write_binding.pImageInfo       = nullptr;
                    temp.descriptor_write_binding.pTexelBufferView = nullptr;
                    temp.bufferInfo                                = {true, buffer_block};
                    shader_temp->update_descriptor_sets.insert({binding_name, temp});
                    return true;
                }
            }
        }
    }
    return false;
}


std::vector<VkDescriptorSet> allocate_descriptor_sets(const entt::entity entity);

#endif //HELLO_MAC_SHADER_COMPONENT_H
