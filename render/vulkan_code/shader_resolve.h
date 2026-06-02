//
// Created by 潘鑫 on 2026/6/2.
//

#ifndef HELLO_MAC_SHADER_RESOLVE_H
#define HELLO_MAC_SHADER_RESOLVE_H

#include <volk.h>
#include <map>

#include "descriptor.h"
#include "vulkan_buffer.h"
#include "vulkan_image.h"

using Push_constant_map = std::map<std::string, VkPushConstantRange>;

struct color_attachment_format {
    uint32_t location;
    // uint32_t size;
    VkFormat format;
    std::string output_name;
};

using Fragment_output_map = std::map<uint32_t, color_attachment_format>;


struct InputAttributeDescription {
    uint32_t location;
    uint32_t binding;
    VkFormat format;
    uint32_t offset;
    uint32_t size;
    std::string name;
};

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


struct shader_need_parameter {
    // std::vector<DescriptorSet_ptr> bindless_descriptor_sets; // descriptor_set 的 共享指针保存点
    // std::vector<DescriptorSet_ptr> global_descriptor_sets;   // descriptor_set 的 共享指针保存点
    std::vector<DescriptorSet_ptr> object_descriptor_sets; // descriptor_set 的 共享指针保存点
    // std::map<std::string, Update_descriptor_binding> update_bindless_descriptor_sets;
    // std::map<std::string, Update_descriptor_binding> update_global_descriptor_sets;
    std::map<std::string, Update_descriptor_binding> update_object_descriptor_sets;
    std::byte push_constant_pool[128];
};

using bindings_map = std::map<uint32_t, binding_resource>;
using sets_map     = std::map<uint32_t, bindings_map>;


struct vk_shader_data {
    std::string shader_key;
    std::vector<VkPipelineShaderStageCreateInfo> pipeline_shader_stage_create_infos;
    std::vector<VkPipelineShaderStageCreateInfo> computer_shader_stage_create_infos;
    VkPrimitiveTopology topology;
    // 再想增加一个组的时候，还是需要到这里来增加
    sets_map bindless_sets_bindings;
    sets_map global_sets_bindings;
    sets_map object_sets_bindings;
    Push_constant_map push_constant_map;
    std::vector<VkDescriptorSetLayout> bindless_set_layout;
    std::vector<VkDescriptorSetLayout> global_descriptor_sets_layout;
    std::vector<VkDescriptorSetLayout> object_descriptor_sets_layout;

    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    std::vector<InputAttributeDescription> vertexAttributes;
    std::vector<VkVertexInputBindingDescription> vertexBindings;
    Fragment_output_map fragment_output_map;


    [[nodiscard]] std::vector<VkVertexInputAttributeDescription> get_vertexAttributes() const {
        std::vector<VkVertexInputAttributeDescription> temp;
        for (const auto &attribute: vertexAttributes) {
            temp.push_back({attribute.location, attribute.binding, attribute.format, attribute.offset});
        }
        return temp;
    }

    Push_constant_map &get_push_constant_map() {
        return push_constant_map;
    }
};

using shader_data = std::shared_ptr<vk_shader_data>;


class VKR_shader_paths {
public:
    VKR_shader_paths(const std::string &vertex_path,
                     const std::string &fragment_path,
                     const std::string &geometry_path,
                     const std::string &computer_path,
                     const VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST) {
        vertex_path_   = vertex_path;
        fragment_path_ = fragment_path;
        geometry_path_ = geometry_path;
        computer_path_ = computer_path;
        topology_      = topology;
    }

    VKR_shader_paths() = delete;

    std::string vertex_path_;
    std::string geometry_path_;
    std::string fragment_path_;
    std::string computer_path_;
    VkPrimitiveTopology topology_ = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;


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

shader_data VKR_shader_init(VKR_shader_paths &shader_paths);

#endif //HELLO_MAC_SHADER_RESOLVE_H
