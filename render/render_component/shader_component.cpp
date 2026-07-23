//
// Created by 潘鑫 on 2026/3/2.
//

#include "shader_component.h"
#include <memory_resource>
#include <pipeline_layout.h>

#include "shader_create.h"
#include "descriptor.h"
#include "pipeline_layout_component.h"
#include "scene_component.h"
#include "sets_and_bindings_layout.h"
#include "transfer_texture_to_gpu.h"
#include "transform_component.h"
#include "VKR_proxy_component.h"
#include "vulkan_backend.h"
#include "vulkan_render_manage.h"


void update_bindings_to_descriptor_sets(const entt::entity entity) {
    auto &vk_s_d_s = Render_entt().get_or_emplace<shader_need_parameter>(entity);
    if (vk_s_d_s.update_object_descriptor_sets.empty()) {
        return;
    }
    allocate_descriptor_sets(entity);
    const Proxy_descriptor_sets &descriptor_sets = get_descriptor_sets(entity);
    update_descriptor_sets(vk_s_d_s.update_object_descriptor_sets, descriptor_sets);
}


void allocate_descriptor_sets(const entt::entity entity) {
    // 这里就全部都是 渲染 某个物体时会 变更的数据了
    // 需要根据是全局还是物体单独的来进行创建了，全局的就获取全局的 descriptor_sets , 然后
    auto &handle = VK_backend::instance();
    if (const auto shader_temp = Render_entt().try_get<shader_data>(entity)) {
        // get_or_emplace 新找到了一个函数，有就返回，没有就创建
        auto &vk_s_d_s = Render_entt().get_or_emplace<shader_need_parameter>(entity);
        if (!(*shader_temp)->object_descriptor_sets_layout.empty()) {
            auto sets_flags = create_descriptor_sets_flags(handle,
                                                           (*shader_temp)->object_sets_bindings);
            vk_s_d_s.object_descriptor_sets = allocate_descriptor_sets(Engine::instance().get_descriptor_pool(),
                                                                       (*shader_temp)->
                                                                       object_descriptor_sets_layout,
                                                                       {});
            // 这里好像每次就把 全部的 都重新申请了 准确的说 是把 某个 set = 0，1，2 的 全部都申请了
            // 另一边，我 只是把 相应的 需要 update 的 数据地址全部 填写到了每个 set 中
            // 只需要解决 set 到问题就好
        }
    }
}

Proxy_descriptor_sets get_descriptor_sets(const entt::entity entity) {
    Proxy_descriptor_sets descriptor_sets; // 这里是需要按照顺序的
    if (const auto vk_s_d_s = Render_entt().try_get<shader_need_parameter>(entity)) {
        if (const auto shader_temp = Render_entt().try_get<shader_data>(entity)) {
            if (!(*shader_temp)->global_descriptor_sets_layout.empty()) {
                auto bindless_descriptor_sets = Engine::instance().get_bindless_descriptor_set();
                auto global_descriptor_sets   = Engine::instance().get_global_descriptor_set();
                for (auto &bindless_descriptor_set: bindless_descriptor_sets) {
                    bindless_descriptor_set = VK_NULL_HANDLE;
                }
                for (auto &global_descriptor_set: global_descriptor_sets) {
                    global_descriptor_set = VK_NULL_HANDLE;
                }
                // 这里清理的原因是 不应该在这里写入
                descriptor_sets.reserve(bindless_descriptor_sets.size() +
                                        global_descriptor_sets.size() +
                                        vk_s_d_s->object_descriptor_sets.size());
                descriptor_sets.insert(descriptor_sets.end(),
                                       bindless_descriptor_sets.begin(),
                                       bindless_descriptor_sets.end());
                descriptor_sets.insert(descriptor_sets.end(),
                                       global_descriptor_sets.begin(),
                                       global_descriptor_sets.end());
                descriptor_sets.insert(descriptor_sets.end(),
                                       vk_s_d_s->object_descriptor_sets.begin(),
                                       vk_s_d_s->object_descriptor_sets.end());
            } else {
                return vk_s_d_s->object_descriptor_sets;
            }
        }
    }
    return descriptor_sets;
}

Proxy_descriptor_sets update_descriptor_sets(const entt::entity entity) {
    Proxy_descriptor_sets descriptor_sets; // 这里是需要按照顺序的
    if (const auto vk_s_d_s = Render_entt().try_get<shader_need_parameter>(entity)) {
        if (const auto shader_temp = Render_entt().try_get<shader_data>(entity)) {
            if (!(*shader_temp)->global_descriptor_sets_layout.empty()) {
                auto bindless_descriptor_sets = Engine::instance().get_bindless_descriptor_set();
                auto global_descriptor_sets   = Engine::instance().get_global_descriptor_set();
                // 这里清理的原因是 不应该在这里写入
                descriptor_sets.reserve(bindless_descriptor_sets.size() +
                                        global_descriptor_sets.size() +
                                        vk_s_d_s->object_descriptor_sets.size());
                descriptor_sets.insert(descriptor_sets.end(),
                                       bindless_descriptor_sets.begin(),
                                       bindless_descriptor_sets.end());
                descriptor_sets.insert(descriptor_sets.end(),
                                       global_descriptor_sets.begin(),
                                       global_descriptor_sets.end());
                descriptor_sets.insert(descriptor_sets.end(),
                                       vk_s_d_s->object_descriptor_sets.begin(),
                                       vk_s_d_s->object_descriptor_sets.end());
            } else {
                return vk_s_d_s->object_descriptor_sets;
            }
        }
    }
    return descriptor_sets;
}

void add_shader(const entt::entity entity, const std::string &vertex_path,
                const std::string &geometry_path,
                const std::string &fragment_path,
                const std::string &computer_path) {
    Logic_entt().emplace<VKR_shader_paths>(entity, vertex_path, geometry_path, fragment_path, computer_path);
    auto &shader_temp = Logic_entt().get<VKR_shader_paths>(entity);
    Logic_entt().emplace<shader_data>(entity, VKR_shader_init(shader_temp));
    logic_update_proxy<shader_data>(entity); // 这步越来越重要了
}

const std::vector<InputAttributeDescription> &get_attribute_description(const entt::entity entity) {
    const auto &shader_temp = Logic_entt().get<shader_data>(entity);
    return shader_temp->vertexAttributes;
}


void global_uniform_buffer_update_function() {
    // 就是检查一下，已经给过 渲染线程，就添加一个 lambda 更新部分内容就好
    // global 相关的内容尽量只能偏移，
    const auto view = Render_entt().view<global_uniform_buffer_update>();
    for (const auto &it: view) {
        update_bindings_to_descriptor_sets(it);
        Render_entt().emplace_or_replace<descriptor_set_update>(it);
        auto lambda = [](const entt::entity entity) {
            if (Render_entt().all_of<Scene_Component>(entity))
                Render_entt().emplace_or_replace<descriptor_set_update>(entity);
        };
        add_recursion_function_to_children(it, lambda);
        Render_entt().remove<global_uniform_buffer_update>(it);
    }
}

void add_bindless_update_tag() {
    auto world_entity = get_world_root();
    // todo: 有问题
    Render_entt().emplace_or_replace<bindless_set_update_detail>(world_entity);
}

void bindless_uniform_sampler2D_update_function() {
    const auto view = Render_entt().view<bindless_set_update_detail>();
    for (const auto &it: view) {
        update_bindings_to_descriptor_sets(it);
        Render_entt().emplace_or_replace<descriptor_set_update>(it); // 不需要，因为 只是增加了内容，不改变 set
        Render_entt().remove<bindless_set_update_detail>(it);
    }
}

void object_parameter_update() {
    const auto view = Render_entt().view<uniform_buffer_update>();
    for (const auto &it: view) {
        update_bindings_to_descriptor_sets(it);
        Render_entt().emplace_or_replace<descriptor_set_update>(it);
        Render_entt().remove<uniform_buffer_update>(it);
    }
}


void descriptor_set_update_function() {
    const auto view = Render_entt().view<descriptor_set_update>();
    // 位置发生了更新，需要讲更新传递出去
    for (const auto it: view) {
        auto temp_des = get_descriptor_sets(it);
        Render_entt().emplace_or_replace<decltype(temp_des)>(it, temp_des);
        Render_entt().remove<descriptor_set_update>(it);
    }
}

void push_constant_update_function() {
    const auto view = Render_entt().view<push_constant_update>();
    // 位置发生了更新，需要讲更新传递出去
    for (const auto it: view) {
        auto parameter = Render_entt().get_or_emplace<shader_need_parameter>(it);
        Render_entt().emplace_or_replace<decltype(parameter)>(it, parameter);
        Render_entt().remove<push_constant_update>(it);
    }
}
