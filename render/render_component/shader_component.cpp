//
// Created by 潘鑫 on 2026/3/2.
//

#include "shader_component.h"
#include <memory_resource>
#include <pipeline_layout.h>

#include "create_shader.h"
#include "descriptor.h"
#include "pipeline_layout_component.h"
#include "scene_component.h"
#include "sets_and_bindings_layout.h"
#include "transfer_texture_to_gpu.h"
#include "VKR_proxy_component.h"
#include "vulkan_backend.h"
#include "vulkan_render_manage.h"


void update_bindings_to_descriptor_sets(const entt::entity entity, const std::string &b_or_g_or_o) {
    // 以 binding 为一个最小数量
    auto &handle = VK_backend::get();
    if (static_cast<uint>(entity) == 1 || static_cast<uint>(entity) == 5) {
        return;
    }

    auto &vk_s_d_s = Render_entt().get_or_emplace<shader_need_parameter>(entity);
    if (b_or_g_or_o == "bindless") {
        vk_s_d_s = Render_entt().get_or_emplace<shader_need_parameter>(get_world_root());
        // allocate_descriptor_sets(instance, "bindless");  // 只放在初次
        const Proxy_descriptor_sets &descriptor_sets = get_descriptor_sets(entity);
        update_descriptor_sets(vk_s_d_s.update_bindless_descriptor_sets, descriptor_sets);
    } else if (b_or_g_or_o == "global") {
        if (vk_s_d_s.update_global_descriptor_sets.empty()) {
            return;
        }
        allocate_descriptor_sets(entity, "global");
        const Proxy_descriptor_sets &descriptor_sets = get_descriptor_sets(entity);
        update_descriptor_sets(vk_s_d_s.update_global_descriptor_sets, descriptor_sets);
    } else if (b_or_g_or_o == "object") {
        if (vk_s_d_s.update_object_descriptor_sets.empty()) {
            return;
        }
        allocate_descriptor_sets(entity, "object");
        const Proxy_descriptor_sets &descriptor_sets = get_descriptor_sets(entity);
        update_descriptor_sets(vk_s_d_s.update_object_descriptor_sets, descriptor_sets);
    }
}

Proxy_descriptor_sets get_global_descriptor_set(const entt::entity entity) {
    Proxy_descriptor_sets global_descriptor_set;

    if (const auto shader_temp = Render_entt().try_get<shader_data>(entity)) {
        if (!(*shader_temp)->global_descriptor_sets_layout.empty()) {
            auto current_entity = entity;
            while (current_entity != entt::null) {
                if (const auto para = Render_entt().try_get<shader_need_parameter>(current_entity)) {
                    if (!para->global_descriptor_sets.empty()) {
                        global_descriptor_set = para->global_descriptor_sets;
                        break;
                    }
                }
                const auto parent_entity = get_parent(current_entity);
                current_entity           = parent_entity;
            }
            // 那就不应该由这里去创建了，而是应该向 父节点 查找，查找到话就拿到并返回
            // 那么要求是什么呢？父节点 和这个节点有相同的着色器
            // 那么是否可以这样呢？ 只要有几何节点，就可以查找自身，使用自身的着色器，
            // 如果自身没有，就使用父节点的着色器
            // 好处是什么呢？只要能分出几何体，就可以绘制，glfw 的物体的 mesh也是可以被解析的
            // 如果一个 mesh 有特殊的材质，就可以专门指定，但是 model 还是用的父节点的数据
        }
    }

    return global_descriptor_set;
}

/**
 * 复制上面的函数，
 * @param entity
 * @return
 */
Proxy_descriptor_sets get_bindless_descriptor_set(const entt::entity entity) {
    Proxy_descriptor_sets global_descriptor_set;


    if (const auto shader_temp = Render_entt().try_get<shader_data>(entity)) {
        if (!(*shader_temp)->bindless_set_layout.empty()) {
            auto current_entity = get_world_root();
            while (current_entity != entt::null) {
                if (const auto para = Render_entt().try_get<shader_need_parameter>(current_entity)) {
                    if (!para->bindless_descriptor_sets.empty()) {
                        global_descriptor_set = para->bindless_descriptor_sets;
                        break;
                    }
                }
                const auto parent_entity = get_parent(current_entity);
                current_entity           = parent_entity;
            }
        }
    }

    return global_descriptor_set;
}


void allocate_descriptor_sets(const entt::entity entity, const std::string &one_binding_name) {
    // 这里就全部都是 渲染 某个物体时会 变更的数据了
    // 需要根据是全局还是物体单独的来进行创建了，全局的就获取全局的 descriptor_sets , 然后
    auto &handle = VK_backend::get();
    if (const auto shader_temp = Render_entt().try_get<shader_data>(entity)) {
        // get_or_emplace 新找到了一个函数，有就返回，没有就创建
        auto &vk_s_d_s = Render_entt().get_or_emplace<shader_need_parameter>(entity);

        if (one_binding_name.find("bindless") != std::string::npos) {
            if (!(*shader_temp)->object_descriptor_sets_layout.empty()) {
                auto sets_flags = create_descriptor_sets_flags(handle,
                                                               (*shader_temp)->bindless_sets_bindings);
                vk_s_d_s.bindless_descriptor_sets = allocate_descriptor_sets(handle,
                                                                             (*shader_temp)->
                                                                             bindless_set_layout,
                                                                             sets_flags);
            }
        } else if (one_binding_name.find("global") != std::string::npos) {
            if (!(*shader_temp)->object_descriptor_sets_layout.empty()) {
                auto sets_flags = create_descriptor_sets_flags(handle,
                                                               (*shader_temp)->global_sets_bindings);
                vk_s_d_s.global_descriptor_sets = allocate_descriptor_sets(handle,
                                                                           (*shader_temp)->
                                                                           global_descriptor_sets_layout,
                                                                           {});
            }
        } else if (one_binding_name.find("object") != std::string::npos) {
            if (!(*shader_temp)->object_descriptor_sets_layout.empty()) {
                auto sets_flags = create_descriptor_sets_flags(handle,
                                                               (*shader_temp)->object_sets_bindings);
                vk_s_d_s.object_descriptor_sets = allocate_descriptor_sets(handle,
                                                                           (*shader_temp)->
                                                                           object_descriptor_sets_layout,
                                                                           {});
            }
        }
    }
}

Proxy_descriptor_sets get_descriptor_sets(const entt::entity entity) {
    Proxy_descriptor_sets descriptor_sets; // 这里是需要按照顺序的
    if (const auto vk_s_d_s = Render_entt().try_get<shader_need_parameter>(entity)) {
        if (const auto shader_temp = Render_entt().try_get<shader_data>(entity)) {
            if (!(*shader_temp)->global_descriptor_sets_layout.empty()) {
                auto bindless_descriptor_sets = get_bindless_descriptor_set(entity);
                auto global_descriptor_sets   = get_global_descriptor_set(entity);
                // 先使用下面的直接引用，之后再看怎么获取父节点的全局索引
                // auto &global_descriptor_sets = vk_s_d_s->global_descriptor_sets;
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

shader_data VKR_shader_init(VKR_shader_paths &shader_paths) {
    shader_data shader_data_handle;
    // if (shader_data_handle.get() == nullptr)
    {
        auto &handle = VK_backend::get();
        shader_data_handle = std::make_shared<vk_shader_data>();
        shader_data_handle->pipeline_shader_stage_create_infos = find_graphics_shader_module(handle, shader_paths);
        shader_data_handle->computer_shader_stage_create_infos = find_compute_shader_module(handle, shader_paths);
        shader_data_handle->object_sets_bindings = organize_descriptor_set_and_binding_layouts(shader_paths,
                 shader_data_handle);
        shader_data_handle->shader_key = get_shader_key(shader_paths);
        shader_data_handle->topology   = shader_paths.topology_;
        // 下面这两个对于创建的顺序有点要求，上面的没有顺序要求

        // descriptor_sets_layout 中包含 global 的 set
        // 重要是如果有时候，set = 0 在 global 时应该如何处理


        shader_data_handle->bindless_set_layout =
                create_descriptor_sets_layout(handle,
                                              shader_data_handle->shader_key + "bindless_set",
                                              shader_data_handle->bindless_sets_bindings);
        shader_data_handle->global_descriptor_sets_layout =
                create_descriptor_sets_layout(handle,
                                              shader_data_handle->shader_key + "global_bindings_set",
                                              shader_data_handle->global_sets_bindings);

        shader_data_handle->object_descriptor_sets_layout =
                create_descriptor_sets_layout(handle,
                                              shader_data_handle->shader_key,
                                              shader_data_handle->object_sets_bindings);
        std::vector<VkDescriptorSetLayout> temp;
        temp.reserve(shader_data_handle->object_descriptor_sets_layout.size() +
                     shader_data_handle->bindless_set_layout.size() +
                     shader_data_handle->global_descriptor_sets_layout.size());

        temp.insert(temp.end(),
                    shader_data_handle->bindless_set_layout.begin(),
                    shader_data_handle->bindless_set_layout.end());
        temp.insert(temp.end(),
                    shader_data_handle->global_descriptor_sets_layout.begin(),
                    shader_data_handle->global_descriptor_sets_layout.end());
        temp.insert(temp.end(),
                    shader_data_handle->object_descriptor_sets_layout.begin(),
                    shader_data_handle->object_descriptor_sets_layout.end());


        shader_data_handle->pipeline_layout = create_pipeline_layout(handle, shader_data_handle->shader_key,
                                                                     temp, shader_data_handle->push_constant_map);
    }
    return shader_data_handle;
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
        update_bindings_to_descriptor_sets(it, "global");
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
        update_bindings_to_descriptor_sets(it, "bindless");
        Render_entt().emplace_or_replace<descriptor_set_update>(it); // 不需要，因为 只是增加了内容，不改变 set
        Render_entt().remove<bindless_set_update_detail>(it);
    }
}

void uniform_buffer_update_function() {
    const auto view = Render_entt().view<uniform_buffer_update>();
    for (const auto &it: view) {
        update_bindings_to_descriptor_sets(it, "object");
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
