//
// Created by 潘鑫 on 2026/3/2.
//

#include "shader_component.h"
#include <memory_resource>

#include "create_shader.h"
#include "descriptor.h"
#include "pipeline_layout.h"
#include "scene_component.h"
#include "sets_and_bindings_layout.h"
#include "transfer_texture_to_gpu.h"
#include "vulkan_device_handle.h"


void update_bindings_to_descriptor_sets(const entt::entity entity,
                                        const std::vector<VkDescriptorSet> &descriptor_sets) {
    // 以 binding 为一个最小数量
    if (const auto shader_temp = g_entt().try_get<VKR_shader>(entity)) {
        auto &handle = VK_handle::get();
        char stack_memory_pool[1024];
        std::pmr::monotonic_buffer_resource pool{stack_memory_pool, sizeof(stack_memory_pool)};
        std::pmr::polymorphic_allocator<std::byte> alloc{&pool};

        std::vector<VkWriteDescriptorSet> descriptor_write_bindings{};
        descriptor_write_bindings.resize(shader_temp->update_descriptor_sets.size());
        size_t i = 0;
        for (auto &[name,binding_update]: shader_temp->update_descriptor_sets) {
            descriptor_write_bindings[i]        = binding_update.descriptor_write_binding;
            descriptor_write_bindings[i].dstSet = descriptor_sets[binding_update.dstSet];
            if (binding_update.bufferInfo.first) {
                const auto buffer_info = reinterpret_cast<VkDescriptorBufferInfo *>(alloc.
                    allocate(sizeof(VkDescriptorBufferInfo)));
                buffer_info->buffer                      = binding_update.bufferInfo.second->get_buffer_handle();
                buffer_info->offset                      = binding_update.bufferInfo.second->offset_;
                buffer_info->range                       = binding_update.bufferInfo.second->size_;
                descriptor_write_bindings[i].pBufferInfo = buffer_info; // 一个需要转换的问题
            } else if (binding_update.imageInfo.first) {
                descriptor_write_bindings[i].pImageInfo = &binding_update.imageInfo.second;;
            } else if (binding_update.TexelBufferView.first) {
                descriptor_write_bindings[i].pTexelBufferView = &binding_update.TexelBufferView.second;
            }
            ++i;
        }
        vkUpdateDescriptorSets(handle.get_device(),
                               static_cast<uint32_t>(descriptor_write_bindings.size()),
                               descriptor_write_bindings.data(),
                               0,
                               nullptr);
    }
}

std::vector<VkDescriptorSet> get_global_descriptor_set(const entt::entity entity) {
    std::vector<VkDescriptorSet> global_descriptor_set;
    auto &handle = VK_handle::get();

    if (const auto shader_temp = g_entt().try_get<VKR_shader>(entity)) {
        if (!shader_temp->shader_data_handle->global_descriptor_sets_layout.empty()) {
            auto sets_flags = create_descriptor_sets_flags(handle,
                                                           shader_temp->shader_data_handle->global_bindings_set);
            global_descriptor_set = allocate_descriptor_sets(handle,
                                                             shader_temp->shader_data_handle->
                                                             global_descriptor_sets_layout,
                                                             sets_flags);
        }
    }
    //
    // if (!shader_temp->shader_data_handle->global_descriptor_sets_layout.empty()) {
    //     auto current_entity = entity;
    //     while (true) {
    //         if (const auto parent_entity = get_parent(current_entity); parent_entity != entt::null) {
    //             if (const auto shader_parent = g_entt().try_get<VKR_shader>(entity)) {
    //                 if (!shader_parent->shader_data_handle->global_bindings_set.empty()) {
    //                     auto &global_bindings_set = shader_parent->shader_data_handle->global_bindings_set;
    //                     break;
    //                 }
    //             }
    //             current_entity = parent_entity;
    //         } else if (parent_entity == entt::null) {
    //             break;
    //         }
    //     }
    //     // 那就不应该由这里去创建了，而是应该向 父节点 查找，查找到话就拿到并返回
    //     // 那么要求是什么呢？父节点 和这个节点有相同的着色器
    //     // 那么是否可以这样呢？ 只要有几何节点，就可以查找自身，使用自身的着色器，
    //     // 如果自身没有，就使用父节点的着色器
    //     // 好处是什么呢？只要能分出几何体，就可以绘制，glfw 的物体的 mesh也是可以被解析的
    //     // 如果一个 mesh 有特殊的材质，就可以专门指定，但是 model 还是用的父节点的数据
    // }

    return global_descriptor_set;
}


std::vector<VkDescriptorSet> allocate_descriptor_sets(const entt::entity entity) {
    // 这里就全部都是 渲染 某个物体时会 变更的数据了
    // 需要根据是全局还是物体单独的来进行创建了，全局的就获取全局的 descriptor_sets , 然后
    std::vector<VkDescriptorSet> descriptor_sets; // 这里是需要按照顺序的
    auto &handle = VK_handle::get();
    if (const auto shader_temp = g_entt().try_get<VKR_shader>(entity)) {
        g_entt().emplace_or_replace<vk_shader_descriptor_sets>(entity);
        auto &vk_s_d_s = g_entt().get<vk_shader_descriptor_sets>(entity);

        vk_s_d_s.global_descriptor_sets = get_global_descriptor_set(entity);
        std::vector<VkDescriptorSet> object_descriptor_sets;

        if (!shader_temp->shader_data_handle->model_descriptor_sets_layout.empty()) {
            // 只是一个物体，查找当前物体的参数
            auto sets_flags = create_descriptor_sets_flags(handle,
                                                           shader_temp->shader_data_handle->model_sets_bindings);

            vk_s_d_s.model_descriptor_sets = allocate_descriptor_sets(handle,
                                                                      shader_temp->shader_data_handle->
                                                                      model_descriptor_sets_layout,
                                                                      sets_flags);
        }
    }

    return descriptor_sets;
}

std::vector<VkDescriptorSet> get_descriptor_sets(const entt::entity entity) {
    std::vector<VkDescriptorSet> descriptor_sets; // 这里是需要按照顺序的
    if (auto vk_s_d_s = g_entt().try_get<vk_shader_descriptor_sets>(entity)) {
        descriptor_sets.reserve(vk_s_d_s->global_descriptor_sets.size() + vk_s_d_s->model_descriptor_sets.size());
        descriptor_sets.insert(descriptor_sets.end(),
                               vk_s_d_s->global_descriptor_sets.begin(),
                               vk_s_d_s->global_descriptor_sets.end());
        descriptor_sets.insert(descriptor_sets.end(),
                               vk_s_d_s->model_descriptor_sets.begin(),
                               vk_s_d_s->model_descriptor_sets.end());
    }
    return descriptor_sets;
}

bool VKR_shader::init() {
    if (shader_data_handle == nullptr) {
        auto &handle                                           = VK_handle::get();
        shader_data_handle                                     = std::make_shared<vk_shader_data>();
        shader_data_handle->pipeline_shader_stage_create_infos = find_graphics_shader_module(handle, *this);
        shader_data_handle->model_sets_bindings                = organize_descriptor_set_and_binding_layouts(*this);
        shader_data_handle->shader_key                         = get_shader_key(*this);
        // 下面这两个对于创建的顺序有点要求，上面的没有顺序要求

        // descriptor_sets_layout 中包含 global 的 set
        // 重要是如果有时候，set = 0 在 global 时应该如何处理
        shader_data_handle->global_descriptor_sets_layout =
                create_descriptor_sets_layout(handle,
                                              shader_data_handle->shader_key + "global_bindings_set",
                                              shader_data_handle->global_bindings_set);

        shader_data_handle->model_descriptor_sets_layout =
                create_descriptor_sets_layout(handle,
                                              shader_data_handle->shader_key,
                                              shader_data_handle->model_sets_bindings);
        std::vector<VkDescriptorSetLayout> temp;
        temp.reserve(shader_data_handle->model_descriptor_sets_layout.size() +
                     shader_data_handle->global_descriptor_sets_layout.size());

        temp.insert(temp.end(),
                    shader_data_handle->global_descriptor_sets_layout.begin(),
                    shader_data_handle->global_descriptor_sets_layout.end());
        temp.insert(temp.end(),
                    shader_data_handle->model_descriptor_sets_layout.begin(),
                    shader_data_handle->model_descriptor_sets_layout.end());


        shader_data_handle->pipeline_layout = create_pipeline_layout(handle, shader_data_handle->shader_key,
                                                                     temp);
    } else {
    }

    return true;
}
