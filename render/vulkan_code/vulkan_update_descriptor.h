//
// Created by 潘鑫 on 2026/3/27.
//

#ifndef HELLO_MAC_VULKAN_UPDATE_DESCRIPTOR_H
#define HELLO_MAC_VULKAN_UPDATE_DESCRIPTOR_H
#include <string>
#include <volk.h>

#include "descriptor.h"
#include "transfer_texture_to_gpu.h"
#include "vulkan_buffer.h"
#include "vulkan_image.h"


struct binding_resource {
    VkDescriptorSetLayoutBinding LayoutBinding{};
    std::string binding_name;
    std::string resource_type; //  "uniform", "uniform sampler2D", "buffer", "uniform sampler" "uniform texture2D"
    std::string shaderStage;
    size_t uniform_buffer_size    = 0;
    VkDescriptorBindingFlags flag = 0;
};

struct Update_descriptor_binding {
    std::string binding_name;
    std::string resource_type;
    uint32_t dstSet                               = 0;
    VkWriteDescriptorSet descriptor_write_binding = {};

    std::pair<bool, VKR_buffer_block_ptr> bufferInfo = {};
    std::pair<bool, VKR_buffer_ptr> SSBO_bufferInfo  = {};
    std::pair<bool, Texture_parameter> texture_info;
    // Texel Buffer 本质上是 Buffer，但它像 Image 一样拥有 格式（Format） 信息
    std::pair<bool, VkBufferView> TexelBufferView;
};


struct Parameter_used {
    std::vector<DescriptorSet_ptr> bindless_descriptor_sets; // descriptor_set 的 共享指针保存点
    std::vector<DescriptorSet_ptr> global_descriptor_sets;   // descriptor_set 的 共享指针保存点
    std::vector<DescriptorSet_ptr> object_descriptor_sets;   // descriptor_set 的 共享指针保存点
    std::map<std::string, Update_descriptor_binding> update_bindless_descriptor_sets;
    std::map<std::string, Update_descriptor_binding> update_global_descriptor_sets;
    std::map<std::string, Update_descriptor_binding> update_object_descriptor_sets;
    std::byte push_constant_pool[128];
};

using bindings_map = std::map<uint32_t, binding_resource>;
using sets_map     = std::map<uint32_t, bindings_map>;


#define Update_descriptor_binding_fixed_temp  \
Update_descriptor_binding temp     = {};\
temp.binding_name                   = binding_name;\
temp.resource_type                  = info.resource_type;\
temp.dstSet                         = set_value;\
temp.descriptor_write_binding.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; \
temp.descriptor_write_binding.dstBinding       = binding_value;\
temp.descriptor_write_binding.dstArrayElement  = 0; \
temp.descriptor_write_binding.descriptorCount  = 1;\
temp.descriptor_write_binding.pBufferInfo      = nullptr;\
temp.descriptor_write_binding.pImageInfo       = nullptr;\
temp.descriptor_write_binding.pTexelBufferView = nullptr;


template<typename T1>
bool set_render_parameter(sets_map &sets_map_in_for,
                          std::map<std::string, Update_descriptor_binding> &update_descriptor_write,
                          const std::string &binding_name,
                          T1 &binding_data) {
    for (auto const &[set_value, bindings_map]: sets_map_in_for) {
        for (const auto &[binding_value, info]: bindings_map) {
            if (info.binding_name == binding_name && info.resource_type == "uniform buffer") {
                VKR_buffer_block_ptr buffer_block = copy_data_to_gpu_buffer(binding_data);
                Update_descriptor_binding_fixed_temp;
                temp.descriptor_write_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                temp.bufferInfo                              = {true, buffer_block};
                update_descriptor_write[binding_name]        = temp;
                return true;
            } else if (info.binding_name == binding_name && info.resource_type == "storage buffer") {
                if constexpr (std::is_same_v<std::decay_t<T1>, VKR_buffer_ptr>) {
                    Update_descriptor_binding_fixed_temp;
                    temp.descriptor_write_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    temp.SSBO_bufferInfo                         = {true, binding_data};
                    update_descriptor_write[binding_name]        = temp;
                    return true;
                }
            } else if (info.binding_name == binding_name && info.resource_type == "uniform sampler2D") {
                // using NoRef = std::remove_reference_t<T1>;
                // 有需要的时候可以去除引用
                if constexpr (std::is_same_v<std::decay_t<T1>, std::string> ||
                              std::is_same_v<std::decay_t<T1>, const char *>) {
                    auto &handle = VK_backend::get();
                    auto texture = create_textures_to_gpu(handle, binding_data);
                    if (texture.has_value()) {
                        Update_descriptor_binding_fixed_temp;
                        temp.descriptor_write_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        temp.texture_info                            = {true, texture.value()};
                        update_descriptor_write[binding_name]        = temp;
                        return true;
                    }
                } else if constexpr (std::is_same_v<std::decay_t<T1>, std::optional<Texture_parameter> >) {
                    if (binding_data.has_value()) {
                        Update_descriptor_binding_fixed_temp;
                        temp.descriptor_write_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        temp.texture_info                            = {true, binding_data.value()};
                        update_descriptor_write[binding_name]        = temp;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}
#undef Update_descriptor_binding_fixed_temp

inline bool add_texture_data_detail(sets_map &sets_map_in_for,
                                    std::map<std::string, Update_descriptor_binding> &update_,
                                    const std::string &binding_name,
                                    const std::string &picture_path) {
    for (auto const &[set_value, bindings_map]: sets_map_in_for) {
        for (const auto &[binding_value, info]: bindings_map) {
            if (info.binding_name == binding_name && info.resource_type == "uniform sampler2D") {
                auto &handle = VK_backend::get();
                auto texture = create_textures_to_gpu(handle, picture_path);
                if (texture.has_value()) {
                    Update_descriptor_binding temp      = {};
                    temp.binding_name                   = binding_name;
                    temp.resource_type                  = info.resource_type;
                    temp.dstSet                         = set_value;
                    temp.descriptor_write_binding.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    // temp.descriptor_write_bindings.dstSet           = descriptor_sets[0];
                    temp.descriptor_write_binding.dstBinding       = binding_value;
                    temp.descriptor_write_binding.dstArrayElement  = 0;
                    temp.descriptor_write_binding.descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    temp.descriptor_write_binding.descriptorCount  = 1;
                    temp.descriptor_write_binding.pBufferInfo      = nullptr;
                    temp.descriptor_write_binding.pImageInfo       = nullptr;
                    temp.descriptor_write_binding.pTexelBufferView = nullptr;
                    temp.texture_info                              = {true, texture.value()};
                    update_[binding_name]                          = temp;
                    return true;
                } else {
                    return false;
                }
            }
        }
    }
    return false;
}


void update_descriptor_sets(std::map<std::string, Update_descriptor_binding> &update_descriptor_sets,
                            const std::vector<DescriptorSet_ptr> &descriptor_sets);

#endif //HELLO_MAC_VULKAN_UPDATE_DESCRIPTOR_H
