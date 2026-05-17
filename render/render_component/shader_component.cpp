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
    if (const auto shader_temp = Logic_entt().try_get<VKR_shader_paths>(entity)) {
        auto &handle = VK_backend::get();

        auto &vk_s_d_s = Logic_entt().get_or_emplace<Parameter_used>(entity);
        if (b_or_g_or_o == "bindless") {
            vk_s_d_s = Logic_entt().get_or_emplace<Parameter_used>(get_world_root());
            // allocate_descriptor_sets(instance, "bindless");  // 只放在初次
            const std::vector<DescriptorSet_ptr> &descriptor_sets = get_descriptor_sets(entity);
            update_descriptor_sets(vk_s_d_s.update_bindless_descriptor_sets, descriptor_sets);
        } else if (b_or_g_or_o == "global") {
            if (vk_s_d_s.update_global_descriptor_sets.empty()) {
                return;
            }
            allocate_descriptor_sets(entity, "global");
            const std::vector<DescriptorSet_ptr> &descriptor_sets = get_descriptor_sets(entity);
            update_descriptor_sets(vk_s_d_s.update_global_descriptor_sets, descriptor_sets);
        } else if (b_or_g_or_o == "object") {
            if (vk_s_d_s.update_object_descriptor_sets.empty()) {
                return;
            }
            allocate_descriptor_sets(entity, "object");
            const std::vector<DescriptorSet_ptr> &descriptor_sets = get_descriptor_sets(entity);
            update_descriptor_sets(vk_s_d_s.update_object_descriptor_sets, descriptor_sets);
        }
    }
}

std::vector<DescriptorSet_ptr> get_global_descriptor_set(const entt::entity entity) {
    std::vector<DescriptorSet_ptr> global_descriptor_set;

    if (const auto shader_temp = Logic_entt().try_get<std::shared_ptr<vk_shader_data> >(entity)) {
        if (!(*shader_temp)->global_descriptor_sets_layout.empty()) {
            auto current_entity = entity;
            while (current_entity != entt::null) {
                if (const auto para = Logic_entt().try_get<Parameter_used>(current_entity)) {
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
std::vector<DescriptorSet_ptr> get_bindless_descriptor_set(const entt::entity entity) {
    std::vector<DescriptorSet_ptr> global_descriptor_set;

    if (const auto shader_temp = Logic_entt().try_get<std::shared_ptr<vk_shader_data> >(entity)) {
        if (!(*shader_temp)->bindless_set_layout.empty()) {
            auto current_entity = get_world_root();
            while (current_entity != entt::null) {
                if (const auto para = Logic_entt().try_get<Parameter_used>(current_entity)) {
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
    if (const auto shader_temp = Logic_entt().try_get<std::shared_ptr<vk_shader_data> >(entity)) {
        // get_or_emplace 新找到了一个函数，有就返回，没有就创建
        auto &vk_s_d_s = Logic_entt().get_or_emplace<Parameter_used>(entity);

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

std::vector<DescriptorSet_ptr> get_descriptor_sets(const entt::entity entity) {
    std::vector<DescriptorSet_ptr> descriptor_sets; // 这里是需要按照顺序的
    if (const auto vk_s_d_s = Logic_entt().try_get<Parameter_used>(entity)) {
        if (const auto shader_temp = Logic_entt().try_get<std::shared_ptr<vk_shader_data> >(entity)) {
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

std::shared_ptr<vk_shader_data> VKR_shader_init(VKR_shader_paths &shader_paths) {
    std::shared_ptr<vk_shader_data> shader_data_handle;
    if (shader_data_handle.get() == nullptr) {
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
    } else {
    }

    return shader_data_handle;
}


void global_uniform_buffer_update_function() {
    // 就是检查一下，已经给过 渲染线程，就添加一个 lambda 更新部分内容就好
    // global 相关的内容尽量只能偏移，
    const auto view = Logic_entt().view<global_uniform_buffer_update>();
    for (const auto &it: view) {
        update_bindings_to_descriptor_sets(it, "global");
        Logic_entt().emplace_or_replace<descriptor_set_update>(it);
        auto lambda = [](const entt::entity entity) {
            if (Logic_entt().all_of<Scene_Component>(entity))
                Logic_entt().emplace_or_replace<descriptor_set_update>(entity);
        };
        add_recursion_function_to_children(it, lambda);
        Logic_entt().remove<global_uniform_buffer_update>(it);
    }
}

void add_bindless_update_tag() {
    auto world_entity = get_world_root();
    Logic_entt().emplace_or_replace<bindless_set_update_detail>(world_entity);
}

void bindless_uniform_sampler2D_update_function() {
    const auto view = Logic_entt().view<bindless_set_update_detail>();
    for (const auto &it: view) {
        update_bindings_to_descriptor_sets(it, "bindless");
        Logic_entt().emplace_or_replace<descriptor_set_update>(it); // 不需要，因为 只是增加了内容，不改变 set
        Logic_entt().remove<bindless_set_update_detail>(it);
    }
}

void uniform_buffer_update_function() {
    const auto view = Logic_entt().view<uniform_buffer_update>();
    for (const auto &it: view) {
        update_bindings_to_descriptor_sets(it, "object");
        Logic_entt().emplace_or_replace<descriptor_set_update>(it);
        Logic_entt().remove<uniform_buffer_update>(it);
    }
}


void descriptor_set_update_function() {
    const auto view = Logic_entt().view<descriptor_set_update>();
    // 位置发生了更新，需要讲更新传递出去
    for (const auto it: view) {
        auto temp_des = get_descriptor_sets(it);

        logic_update_proxy_descriptor_sets(it, temp_des);

        Logic_entt().remove<descriptor_set_update>(it);
    }
}

void push_constant_update_function() {
    const auto view = Logic_entt().view<push_constant_update>();
    // 位置发生了更新，需要讲更新传递出去
    for (const auto it: view) {
        auto &parameter = Logic_entt().get_or_emplace<Parameter_used>(it);

        std::byte push_constant_pool[128];
        memcpy(push_constant_pool, parameter.push_constant_pool, 128);

        //
        // if (const auto render = Logic_entt().try_get<Proxy_entity>(it)) {
        //     const auto entity_temp = render->entity_;
        //     auto lambda            = [entity_temp, push_constant_pool]() {
        //         if (const auto proxy = Render_entt().try_get<VKR_object_proxy>(entity_temp))
        //             memcpy(proxy->push_constants_pool, push_constant_pool, 128);
        //     };
        //     vk_render_queue::instance().render_update_entt(lambda);
        // }

        Logic_entt().remove<push_constant_update>(it);
    }
}
