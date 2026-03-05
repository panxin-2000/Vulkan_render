//
// Created by 潘鑫 on 2026/3/2.
//

#ifndef HELLO_MAC_SHADER_COMPONENT_H
#define HELLO_MAC_SHADER_COMPONENT_H
#include <map>
#include <global_singleton.h>
#include "vulkan_buffer.h"


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
    sets_map global_bindings_set;
    sets_map model_sets_bindings;
    std::vector<VkDescriptorSetLayout> global_descriptor_sets_layout;
    std::vector<VkDescriptorSetLayout> model_descriptor_sets_layout;

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


struct vk_shader_descriptor_sets {
    std::vector<VkDescriptorSet> global_descriptor_sets;
    std::vector<VkDescriptorSet> model_descriptor_sets;
    std::map<std::string, Update_descriptor_binding> update_descriptor_sets;
};

class VKR_shader {
public:
    VKR_shader(const std::string &vertex_path,
               const std::string &fragment_path,
               const std::string &geometry_path,
               const std::string &computer_path) {
        vertex_path_   = vertex_path;
        fragment_path_ = fragment_path;
        geometry_path_ = geometry_path;
        computer_path_ = computer_path;
        init();
    }

    VKR_shader() = delete;

    std::string vertex_path_;
    std::string geometry_path_;
    std::string fragment_path_;
    std::string computer_path_;
    std::shared_ptr<vk_shader_data> shader_data_handle = nullptr;
    std::map<std::string, Update_descriptor_binding> update_descriptor_sets;

    bool init();

    void set_vertex_shader(const std::string &path) {
        vertex_path_ = path;
    }

    void set_fragment_shader(const std::string &path) {
        fragment_path_ = path;
    }

    void set_geometry_shader(const std::string &path) {
        geometry_path_ = path;
    }
};


void update_bindings_to_descriptor_sets(const entt::entity entity,
                                        const std::vector<VkDescriptorSet> &descriptor_sets);

#include "update_push_constants_data.h"

template<typename T1>
bool add_uniform_buffer_data(const entt::entity entity, const std::string &binding_name, T1 binding_data) {
    if (const auto shader_temp = g_entt().try_get<VKR_shader>(entity)) {
        sets_map *sets_map_in_for = nullptr;
        if (binding_name.find("global") != std::string::npos) {
            sets_map_in_for = &shader_temp->shader_data_handle->global_bindings_set;
        } else {
            sets_map_in_for = &shader_temp->shader_data_handle->model_sets_bindings;
        }
        for (auto const &[set_value, bindings_map]: *sets_map_in_for) {
            for (const auto &[binding_value, info]: bindings_map) {
                if (info.binding_name == binding_name && info.resource_type == "uniform buffer") {
                    auto buffer_block                   = copy_data_to_gpu_buffer(binding_data);
                    Update_descriptor_binding temp      = {};
                    temp.binding_name                   = binding_name;
                    temp.resource_type                  = info.resource_type;
                    temp.dstSet                         = set_value;
                    temp.descriptor_write_binding.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    // temp.descriptor_write_bindings.dstSet           = descriptor_sets[0];
                    temp.descriptor_write_binding.dstBinding          = binding_value;
                    temp.descriptor_write_binding.dstArrayElement     = 0;
                    temp.descriptor_write_binding.descriptorType      = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    temp.descriptor_write_binding.descriptorCount     = 1;
                    temp.descriptor_write_binding.pBufferInfo         = nullptr;
                    temp.descriptor_write_binding.pImageInfo          = nullptr;
                    temp.descriptor_write_binding.pTexelBufferView    = nullptr;
                    temp.bufferInfo                                   = {true, buffer_block};
                    shader_temp->update_descriptor_sets[binding_name] = temp;
                    return true;
                }
            }
        }
    }
    return false;
}


std::vector<VkDescriptorSet> allocate_descriptor_sets(const entt::entity entity);

#endif //HELLO_MAC_SHADER_COMPONENT_H
