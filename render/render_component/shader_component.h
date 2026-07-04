//
// Created by 潘鑫 on 2026/3/2.
//

#ifndef HELLO_MAC_SHADER_COMPONENT_H
#define HELLO_MAC_SHADER_COMPONENT_H
#include <map>
#include <global_singleton.h>

#include "create_texture.h"
#include "descriptor.h"
#include "../engine.h"
#include "sync_proxy_to_render_thread.h"
#include "vulkan_buffer.h"
#include "update_push_constants_data.h"


#include "vulkan_update_descriptor.h"
#include "VKR_proxy_component.h"


#include "shader_resolve.h"


void add_shader(const entt::entity entity,
                const std::string &vertex_path,
                const std::string &geometry_path,
                const std::string &fragment_path,
                const std::string &computer_path);

const std::vector<InputAttributeDescription> &get_attribute_description(const entt::entity entity);

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
bool render_render_parameter(const entt::entity entity, const std::string &binding_name, T1 &binding_data) {
    auto &shader_data_ref = Render_entt().get<shader_data>(entity);
    auto &parameter       = Render_entt().get_or_emplace<shader_need_parameter>(entity);
    // if (binding_name.find("global") != std::string::npos) {
    //     set_render_parameter(shader_data_ref->global_sets_bindings,
    //                          parameter.update_global_descriptor_sets, binding_name,
    //                          binding_data);
    //     Render_entt().emplace_or_replace<global_uniform_buffer_update>(entity);
    //     return true;
    // } else
    {
        set_render_parameter(shader_data_ref->object_sets_bindings,
                             parameter.update_object_descriptor_sets, binding_name,
                             binding_data);
        Render_entt().emplace_or_replace<uniform_buffer_update>(entity);
        return true;
    }
    return false;
}

template<typename T1>
bool set_render_parameter(const entt::entity entity, const std::string &binding_name, T1 &binding_data) {
    if (auto proxy_entity = get_proxy_entity(entity); proxy_entity != entt::null) {
        auto lambda = [ proxy_entity,binding_name, binding_data ]() {
            render_render_parameter(proxy_entity, binding_name, binding_data);
        };
        vk_render_queue::instance().render_update_entt(lambda);
    };
    return true;
}

template<typename T1>
bool render_push_constant_parameter(const entt::entity entity, const std::string &binding_name, T1 &binding_data) {
    const auto &shader_data_ref = Render_entt().get<shader_data>(entity);
    auto &parameter             = Render_entt().get_or_emplace<shader_need_parameter>(entity);
    for (auto &[name,value]: shader_data_ref->push_constant_map) {
        if (name == binding_name && sizeof(T1) <= value.size) {
            memcpy(parameter.push_constant_pool + value.offset, &binding_data, sizeof(T1));
            Render_entt().emplace_or_replace<push_constant_update>(entity);
            return true;
        }
    }
    return false;
}

template<typename T1>
bool set_push_constant_parameter(const entt::entity entity, const std::string &binding_name, T1 &binding_data) {
    if (auto proxy_entity = get_proxy_entity(entity); proxy_entity != entt::null) {
        auto lambda = [ proxy_entity, binding_name, binding_data ]() {
            render_push_constant_parameter(proxy_entity, binding_name, binding_data);
        };
        vk_render_queue::instance().render_update_entt(lambda);
    };
    return true;
}

template<typename T1>
VKR_buffer_block_ptr set_render_push_constant_parameter(const entt::entity entity, const std::string &binding_name,
                                                        T1 binding_data) {
    auto buffer_block = copy_data_to_gpu_buffer(binding_data);
    return buffer_block;
}

void allocate_descriptor_sets(const entt::entity entity, const std::string &one_binding_name);

Proxy_descriptor_sets get_descriptor_sets(const entt::entity entity);

Proxy_descriptor_sets update_descriptor_sets(const entt::entity entity);


void descriptor_set_update_function();

void uniform_buffer_update_function();

void global_uniform_buffer_update_function();

void add_bindless_update_tag();

void bindless_uniform_sampler2D_update_function();

void push_constant_update_function();


#endif //HELLO_MAC_SHADER_COMPONENT_H
