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
#include "absl/hash/hash.h" // 引入 Google Abseil 头文件


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
};

struct shader_constant_parameter {
    std::byte push_constant_pool[128];
};


using bindings_map = std::map<uint32_t, binding_resource>;
using sets_map     = std::map<uint32_t, bindings_map>;


class vk_shader_data {
public:
    std::string shader_key;
    std::vector<VkPipelineShaderStageCreateInfo> pipeline_shader_stage_create_infos;
    std::vector<VkPipelineShaderStageCreateInfo> computer_shader_stage_create_infos;
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    // 再想增加一个组的时候，还是需要到这里来增加
    sets_map bindless_sets_bindings;
    sets_map global_sets_bindings;
    sets_map object_sets_bindings;
    Push_constant_map push_constant_map;
    std::vector<VkDescriptorSetLayout> bindless_set_layout;
    std::vector<VkDescriptorSetLayout> global_descriptor_sets_layout;
    std::vector<VkDescriptorSetLayout> object_descriptor_sets_layout;

    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    VkPipeline pipeline_t            = VK_NULL_HANDLE;
    std::vector<InputAttributeDescription> vertexAttributes;
    std::vector<VkVertexInputBindingDescription> vertexBindings;
    Fragment_output_map fragment_output_map;
    VkFormat depthAttachmentFormat;
    VkFormat stencilAttachmentFormat;


    [[nodiscard]] std::vector<VkVertexInputAttributeDescription> get_vertexAttributes() const;

    Push_constant_map &get_push_constant_map() {
        return push_constant_map;
    }

    ~vk_shader_data();
};

using Shader_data = std::shared_ptr<vk_shader_data>;

#ifndef SHADER_BASE_DIR
#define SHADER_BASE_DIR "/Users/panxin/CLionProjects/hello_mac/render/shader/"
#endif


class VKR_shader_paths {
public:
    VKR_shader_paths(const std::string &vertex_path,
                     const std::string &fragment_path,
                     const std::string &geometry_path,
                     const std::string &compute_path,
                     const VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                     VkFormat depthAttachmentFormat     = VK_FORMAT_UNDEFINED,
                     VkFormat stencilAttachmentFormat   = VK_FORMAT_UNDEFINED);

    VKR_shader_paths() = delete;

    std::string vertex_path_;
    std::string geometry_path_;
    std::string fragment_path_;
    std::string compute_path_;
    VkPrimitiveTopology topology_ = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkFormat depthAttachmentFormat_;
    VkFormat stencilAttachmentFormat_;


    // 利用 tuple 快速比较
    bool operator==(const VKR_shader_paths &other) const = default;

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
        compute_path_ = path;
    }


    template<typename H>
    friend H AbslHashValue(H state, const VKR_shader_paths &sp) {
        // 直接使用 H::combine 把所有成员丢进去，它支持任意数量、任意类型的参数！
        // 并且完美支持原生枚举（如 VkPrimitiveTopology），不需要进行 static_cast 转换
        return H::combine(std::move(state),
                          sp.vertex_path_,
                          sp.geometry_path_,
                          sp.fragment_path_,
                          sp.compute_path_,
                          sp.topology_,
                          sp.depthAttachmentFormat_,
                          sp.stencilAttachmentFormat_);
    }
};


Shader_data VKR_shader_init(const VKR_shader_paths &shader_paths);

#endif //HELLO_MAC_SHADER_RESOLVE_H
