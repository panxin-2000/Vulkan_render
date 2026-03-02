//
// Created by 潘鑫 on 2026/1/16.
//

#ifndef HELLO_MAC_LOGIC_RENDER_DATA_H
#define HELLO_MAC_LOGIC_RENDER_DATA_H

#include <map>

#include "APP_utility_mixins.h"
#include "mesh_component.h"
#include "utility.h"


/**
 * vertices_changed         <br>
 * indices_changed          <br>
 * texture_path_changed     <br>
 * texture_name_changed     <br>
 * vertex_path_changed      <br>
 * fragment_path_changed    <br>
 * geometry_path_changed    <br>
 * primitive_type_changed   <br>
 * uniform_buffer_changed   <br>
 */
enum status_change : uint16_t {
    no_change              = 0,
    vertices_changed       = 1 << 0,
    indices_changed        = 1 << 1,
    texture_path_changed   = 1 << 2,
    texture_name_changed   = 1 << 3,
    vertex_path_changed    = 1 << 4,
    fragment_path_changed  = 1 << 5,
    geometry_path_changed  = 1 << 6,
    primitive_type_changed = 1 << 7,
    uniform_buffer_changed = 1 << 8,
};

ENABLE_BITWISE_OPERATORS(status_change)


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

    std::pair<bool, VKR_buffer_block_ptr> bufferInfo;
    std::pair<bool, VkDescriptorImageInfo> imageInfo;
    // Texel Buffer 本质上是 Buffer，但它像 Image 一样拥有 格式（Format） 信息
    std::pair<bool, VkBufferView> TexelBufferView;
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


class Geometry_data : public NonCopyable {
public:
    std::string mesh_path_;

    std::vector<share_block> vertices_vector;
    share_block indices_;


    Geometry_data() = default;

    ~Geometry_data() = default;


    void push_vertices(const share_block &temp) {
        vertices_vector.push_back(temp);
    }


    auto get_indices() const {
        return indices_;
    }

    void set_indices(const share_block &indices) {
        indices_ = indices;
    }
};

Model_mesh create_mesh(Geometry_data &data,
                       std::map<Geometry_data *, mesh_and_share> &map);


#endif //HELLO_MAC_LOGIC_RENDER_DATA_H
