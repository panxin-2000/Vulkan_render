//
// Created by 潘鑫 on 2026/3/2.
//

#ifndef HELLO_MAC_SHADER_COMPONENT_H
#define HELLO_MAC_SHADER_COMPONENT_H
#include <map>
#include <global_singleton.h>

#include "create_texture.h"
#include "descriptor.h"
#include "transform_component.h"
#include "sync_proxy_to_render_thread.h"
#include "transfer_texture_to_gpu.h"
#include "vulkan_buffer.h"
#include "update_push_constants_data.h"


#include "vulkan_update_descriptor.h"

using Push_constant_map = std::map<std::string, VkPushConstantRange>;

struct color_attachment_format {
    uint32_t location;
    // uint32_t size;
    VkFormat format;
    std::string output_name;
};

using Fragment_output_map = std::map<uint32_t, color_attachment_format>;

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
    std::vector<VkVertexInputAttributeDescription> vertexAttributes;
    std::vector<VkVertexInputBindingDescription> vertexBindings;
    Fragment_output_map fragment_output_map;
};


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

std::shared_ptr<vk_shader_data> VKR_shader_init(VKR_shader_paths &shader_paths);

/**
 *
 * @tparam T1
 * @param entity
 * @param binding_name
 * @param binding_data
 *      设置图片时的类型  std::optional<Texture_parameter> &  和 std::string &
 *      设置 uniform buffer 的类型 随机 需要和 shader 中的结构体大小相同
 *      设置 storage buffer 的类型 VKR_buffer_block_ptr &
 *
 * @return
 */
template<typename T1>
bool set_render_parameter(const entt::entity entity, const std::string &binding_name, T1 &binding_data) {
    if (const auto shader_temp = Logic_entt().try_get<VKR_shader_paths>(entity)) {
        if (!Logic_entt().all_of<std::shared_ptr<vk_shader_data> >(entity)) {
            Logic_entt().emplace<std::shared_ptr<vk_shader_data> >(entity, VKR_shader_init(*shader_temp));
        }
        const auto &shader_data = Logic_entt().get<std::shared_ptr<vk_shader_data> >(entity);
        auto &parameter         = Logic_entt().get_or_emplace<Parameter_used>(entity);
        if (binding_name.find("global") != std::string::npos) {
            set_render_parameter(shader_data->global_sets_bindings,
                                 parameter.update_global_descriptor_sets, binding_name,
                                 binding_data);
            Logic_entt().emplace_or_replace<global_uniform_buffer_update>(entity);
            return true;
        } else {
            set_render_parameter(shader_data->object_sets_bindings,
                                 parameter.update_object_descriptor_sets, binding_name,
                                 binding_data);
            Logic_entt().emplace_or_replace<uniform_buffer_update>(entity);
            return true;
        }
    }
    return false;
}


template<typename T1>
bool set_push_constant_parameter(const entt::entity entity, const std::string &binding_name, T1 &binding_data) {
    if (const auto shader_temp = Logic_entt().try_get<VKR_shader_paths>(entity)) {
        if (!Logic_entt().all_of<std::shared_ptr<vk_shader_data> >(entity)) {
            Logic_entt().emplace<std::shared_ptr<vk_shader_data> >(entity, VKR_shader_init(*shader_temp));
        }
        const auto &shader_data = Logic_entt().get<std::shared_ptr<vk_shader_data> >(entity);
        auto &parameter         = Logic_entt().get_or_emplace<Parameter_used>(entity);
        for (auto &[name,value]: shader_data->push_constant_map) {
            if (name == binding_name && sizeof(T1) <= value.size) {
                memcpy(parameter.push_constant_pool + value.offset, &binding_data, sizeof(T1));
                Logic_entt().emplace_or_replace<push_constant_update>(entity);
            }
        }
    }
    return false;
}


template<typename T1>
VKR_buffer_block_ptr set_render_push_constant_parameter(const entt::entity entity, const std::string &binding_name,
                                                        T1 binding_data) {
    auto buffer_block = copy_data_to_gpu_buffer(binding_data);
    return buffer_block;
}

void allocate_descriptor_sets(const entt::entity entity, const std::string &one_binding_name);

std::vector<DescriptorSet_ptr> get_descriptor_sets(const entt::entity entity);


void descriptor_set_update_function();

void uniform_buffer_update_function();

void global_uniform_buffer_update_function();

void add_bindless_update_tag();

void bindless_uniform_sampler2D_update_function();

#endif //HELLO_MAC_SHADER_COMPONENT_H
