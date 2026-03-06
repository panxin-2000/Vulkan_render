//
// Created by 潘鑫 on 2026/3/2.
//

#ifndef HELLO_MAC_SHADER_COMPONENT_H
#define HELLO_MAC_SHADER_COMPONENT_H
#include <map>
#include <global_singleton.h>

#include "descriptor.h"
#include "vulkan_buffer.h"
#include "update_push_constants_data.h"


struct binding_resource {
    VkDescriptorSetLayoutBinding LayoutBinding{};
    std::string binding_name;
    std::string resource_type; //  "uniform", "uniform sampler2D", "buffer", "uniform sampler" "uniform texture2D"
    std::string shaderStage;
    size_t uniform_buffer_size    = 0;
    VkDescriptorBindingFlags flag = 0;
};

using bindings_map = std::map<uint32_t, binding_resource>;
using sets_map     = std::map<uint32_t, bindings_map>;

struct vk_shader_data {
    std::string shader_key;
    std::vector<VkPipelineShaderStageCreateInfo> pipeline_shader_stage_create_infos;

    // 再想增加一个组的时候，还是需要到这里来增加
    sets_map global_sets_bindings;
    sets_map object_sets_bindings;
    std::vector<VkDescriptorSetLayout> global_descriptor_sets_layout;
    std::vector<VkDescriptorSetLayout> object_descriptor_sets_layout;

    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    std::vector<VkVertexInputAttributeDescription> vertexAttributes;
    std::vector<VkVertexInputBindingDescription> vertexBindings;
};

struct Update_descriptor_binding {
    std::string binding_name;
    std::string resource_type;
    uint32_t dstSet                               = 0;
    VkWriteDescriptorSet descriptor_write_binding = {};

    std::pair<bool, VKR_buffer_block_ptr> bufferInfo = {};
    std::pair<bool, VkDescriptorImageInfo> imageInfo;
    // Texel Buffer 本质上是 Buffer，但它像 Image 一样拥有 格式（Format） 信息
    std::pair<bool, VkBufferView> TexelBufferView;
};


struct Parameter_used {
    std::vector<DescriptorSet_ptr> global_descriptor_sets; // descriptor_set 的 共享指针保存点
    std::vector<DescriptorSet_ptr> object_descriptor_sets; // descriptor_set 的 共享指针保存点
    std::map<std::string, Update_descriptor_binding> update_global_descriptor_sets;
    std::map<std::string, Update_descriptor_binding> update_object_descriptor_sets;
};

class VKR_shader_paths {
public:
    VKR_shader_paths(const std::string &vertex_path,
                     const std::string &fragment_path,
                     const std::string &geometry_path,
                     const std::string &computer_path) {
        vertex_path_   = vertex_path;
        fragment_path_ = fragment_path;
        geometry_path_ = geometry_path;
        computer_path_ = computer_path;
    }

    VKR_shader_paths() = delete;

    std::string vertex_path_;
    std::string geometry_path_;
    std::string fragment_path_;
    std::string computer_path_;


    void set_vertex_shader(const std::string &path) {
        vertex_path_ = path;
    }

    void set_fragment_shader(const std::string &path) {
        fragment_path_ = path;
    }

    void set_geometry_shader(const std::string &path) {
        geometry_path_ = path;
    }

    void set_computer_path(const std::string &path) {
        computer_path_ = path;
    }
};

std::shared_ptr<vk_shader_data> VKR_shader_init(VKR_shader_paths &shader_paths);

void update_object_bindings_to_descriptor_sets(const entt::entity entity);

void update_global_bindings_to_descriptor_sets(const entt::entity entity);


template<typename T1>
bool add_uniform_buffer_data_detail(sets_map &sets_map_in_for,
                                    std::map<std::string, Update_descriptor_binding> &update_,
                                    const std::string &binding_name,
                                    T1 binding_data) {
    for (auto const &[set_value, bindings_map]: sets_map_in_for) {
        for (const auto &[binding_value, info]: bindings_map) {
            if (info.binding_name == binding_name && info.resource_type == "uniform buffer") {
                auto buffer_block                   = copy_data_to_gpu_buffer(binding_data);
                Update_descriptor_binding temp      = {};
                temp.binding_name                   = binding_name;
                temp.resource_type                  = info.resource_type;
                temp.dstSet                         = set_value;
                temp.descriptor_write_binding.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                // temp.descriptor_write_bindings.dstSet           = descriptor_sets[0];
                temp.descriptor_write_binding.dstBinding       = binding_value;
                temp.descriptor_write_binding.dstArrayElement  = 0;
                temp.descriptor_write_binding.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                temp.descriptor_write_binding.descriptorCount  = 1;
                temp.descriptor_write_binding.pBufferInfo      = nullptr;
                temp.descriptor_write_binding.pImageInfo       = nullptr;
                temp.descriptor_write_binding.pTexelBufferView = nullptr;
                temp.bufferInfo                                = {true, buffer_block};
                update_[binding_name]                          = temp;
                return true;
            }
        }
    }
    return false;
}


template<typename T1>
bool set_render_parameter(const entt::entity entity, const std::string &binding_name, T1 binding_data) {
    if (const auto shader_temp = g_entt().try_get<VKR_shader_paths>(entity)) {
        const auto &shader_data =
                g_entt().get_or_emplace<std::shared_ptr<vk_shader_data> >(entity, VKR_shader_init(*shader_temp));
        auto &parameter = g_entt().get_or_emplace<Parameter_used>(entity);
        if (binding_name.find("global") != std::string::npos) {
            add_uniform_buffer_data_detail(shader_data->global_sets_bindings,
                                           parameter.update_global_descriptor_sets, binding_name,
                                           binding_data);
            g_entt().emplace_or_replace<global_uniform_buffer_update>(entity);
            return true;
        } else {
            add_uniform_buffer_data_detail(shader_data->object_sets_bindings,
                                           parameter.update_object_descriptor_sets, binding_name,
                                           binding_data);
            g_entt().emplace_or_replace<uniform_buffer_update>(entity);
            return true;
        }
    }
    return false;
}


void allocate_descriptor_sets(const entt::entity entity, const std::string &one_binding_name);

std::vector<DescriptorSet_ptr> get_descriptor_sets(const entt::entity entity);

VkPipeline get_pipeline(const entt::entity entity);

VkPipelineLayout get_pipeline_layout(const entt::entity entity);

#endif //HELLO_MAC_SHADER_COMPONENT_H
