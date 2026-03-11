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
#include "VKR_proxy_component.h"
#include "vulkan_backend.h"


void update_object_bindings_to_descriptor_sets(const entt::entity entity) {
    // 以 binding 为一个最小数量
    if (const auto shader_temp = g_entt().try_get<VKR_shader_paths>(entity)) {
        auto &handle = VK_backend::get();
        char stack_memory_pool[1024];
        std::pmr::monotonic_buffer_resource pool{stack_memory_pool, sizeof(stack_memory_pool)};
        std::pmr::polymorphic_allocator<std::byte> alloc{&pool};

        auto &vk_s_d_s = g_entt().get_or_emplace<Parameter_used>(entity);

        if (vk_s_d_s.update_object_descriptor_sets.empty()) {
            return;
        }
        allocate_descriptor_sets(entity, "object");
        const std::vector<DescriptorSet_ptr> &descriptor_sets = get_descriptor_sets(entity);

        std::vector<VkWriteDescriptorSet> descriptor_write_bindings{};
        descriptor_write_bindings.resize(vk_s_d_s.update_object_descriptor_sets.size());
        size_t i = 0;
        for (auto &[name,binding_update]: vk_s_d_s.update_object_descriptor_sets) {
            descriptor_write_bindings[i]        = binding_update.descriptor_write_binding;
            descriptor_write_bindings[i].dstSet = descriptor_sets[binding_update.dstSet]->get_descriptor_set();
            if (binding_update.bufferInfo.first) {
                const auto buffer_info = reinterpret_cast<VkDescriptorBufferInfo *>(alloc.
                    allocate(sizeof(VkDescriptorBufferInfo)));
                buffer_info->buffer                      = binding_update.bufferInfo.second->get_buffer_handle();
                buffer_info->offset                      = binding_update.bufferInfo.second->offset_;
                buffer_info->range                       = binding_update.bufferInfo.second->size_;
                descriptor_write_bindings[i].pBufferInfo = buffer_info; // 一个需要转换的问题
            } else if (binding_update.texture_info.first) {
                auto image_info = binding_update.texture_info.second.get_Descriptor_Image_Info();
                descriptor_write_bindings[i].pImageInfo = &image_info;
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

void update_global_bindings_to_descriptor_sets(const entt::entity entity) {
    // 以 binding 为一个最小数量
    if (const auto shader_temp = g_entt().try_get<VKR_shader_paths>(entity)) {
        auto &handle = VK_backend::get();
        char stack_memory_pool[1024];
        std::pmr::monotonic_buffer_resource pool{stack_memory_pool, sizeof(stack_memory_pool)};
        std::pmr::polymorphic_allocator<std::byte> alloc{&pool};

        auto &vk_s_d_s = g_entt().get_or_emplace<Parameter_used>(entity);

        if (vk_s_d_s.update_global_descriptor_sets.empty()) {
            return;
        }
        allocate_descriptor_sets(entity, "global");
        const std::vector<DescriptorSet_ptr> &descriptor_sets = get_descriptor_sets(entity);

        std::vector<VkWriteDescriptorSet> descriptor_write_bindings{};
        descriptor_write_bindings.resize(vk_s_d_s.update_global_descriptor_sets.size());
        size_t i = 0;
        for (auto &[name,binding_update]: vk_s_d_s.update_global_descriptor_sets) {
            descriptor_write_bindings[i]        = binding_update.descriptor_write_binding;
            descriptor_write_bindings[i].dstSet = descriptor_sets[binding_update.dstSet]->get_descriptor_set();
            if (binding_update.bufferInfo.first) {
                const auto buffer_info = reinterpret_cast<VkDescriptorBufferInfo *>(alloc.
                    allocate(sizeof(VkDescriptorBufferInfo)));
                buffer_info->buffer                      = binding_update.bufferInfo.second->get_buffer_handle();
                buffer_info->offset                      = binding_update.bufferInfo.second->offset_;
                buffer_info->range                       = binding_update.bufferInfo.second->size_;
                descriptor_write_bindings[i].pBufferInfo = buffer_info; // 一个需要转换的问题
            } else if (binding_update.texture_info.first) {
                auto image_info = binding_update.texture_info.second.get_Descriptor_Image_Info();
                descriptor_write_bindings[i].pImageInfo = &image_info;
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

std::vector<DescriptorSet_ptr> get_global_descriptor_set(const entt::entity entity) {
    std::vector<DescriptorSet_ptr> global_descriptor_set;

    if (const auto shader_temp = g_entt().try_get<std::shared_ptr<vk_shader_data> >(entity)) {
        if (!(*shader_temp)->global_descriptor_sets_layout.empty()) {
            auto current_entity = entity;
            while (current_entity != entt::null) {
                if (const auto para = g_entt().try_get<Parameter_used>(current_entity)) {
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


void allocate_descriptor_sets(const entt::entity entity, const std::string &one_binding_name) {
    // 这里就全部都是 渲染 某个物体时会 变更的数据了
    // 需要根据是全局还是物体单独的来进行创建了，全局的就获取全局的 descriptor_sets , 然后
    auto &handle = VK_backend::get();
    if (const auto shader_temp = g_entt().try_get<std::shared_ptr<vk_shader_data> >(entity)) {
        // get_or_emplace 新找到了一个函数，有就返回，没有就创建
        auto &vk_s_d_s = g_entt().get_or_emplace<Parameter_used>(entity);

        if (one_binding_name.find("global") != std::string::npos) {
            if (!(*shader_temp)->object_descriptor_sets_layout.empty()) {
                auto sets_flags = create_descriptor_sets_flags(handle,
                                                               (*shader_temp)->global_sets_bindings);
                vk_s_d_s.global_descriptor_sets = allocate_descriptor_sets(handle,
                                                                           (*shader_temp)->
                                                                           global_descriptor_sets_layout,
                                                                           {});
            }
        } else {
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
    if (const auto vk_s_d_s = g_entt().try_get<Parameter_used>(entity)) {
        if (const auto shader_temp = g_entt().try_get<std::shared_ptr<vk_shader_data> >(entity)) {
            if (!(*shader_temp)->global_descriptor_sets_layout.empty()) {
                auto global_descriptor_sets = get_global_descriptor_set(entity);
                // 先使用下面的直接引用，之后再看怎么获取父节点的全局索引
                // auto &global_descriptor_sets = vk_s_d_s->global_descriptor_sets;
                descriptor_sets.reserve(global_descriptor_sets.size() + vk_s_d_s->object_descriptor_sets.size());
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
        shader_data_handle->object_sets_bindings = organize_descriptor_set_and_binding_layouts(shader_paths,
                 shader_data_handle);
        shader_data_handle->shader_key = get_shader_key(shader_paths);
        // 下面这两个对于创建的顺序有点要求，上面的没有顺序要求

        // descriptor_sets_layout 中包含 global 的 set
        // 重要是如果有时候，set = 0 在 global 时应该如何处理
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
                     shader_data_handle->global_descriptor_sets_layout.size());

        temp.insert(temp.end(),
                    shader_data_handle->global_descriptor_sets_layout.begin(),
                    shader_data_handle->global_descriptor_sets_layout.end());
        temp.insert(temp.end(),
                    shader_data_handle->object_descriptor_sets_layout.begin(),
                    shader_data_handle->object_descriptor_sets_layout.end());


        shader_data_handle->pipeline_layout = create_pipeline_layout(handle, shader_data_handle->shader_key,
                                                                     temp);
    } else {
    }

    return shader_data_handle;
}

#include "create_pipeline.h"


VkPipeline get_pipeline(const entt::entity entity) {
    auto &handle          = VK_backend::get();
    VkPipeline pipeline_t = VK_NULL_HANDLE;
    if (auto shader_data = g_entt().try_get<std::shared_ptr<vk_shader_data> >(entity)) {
        pipeline_t = find_pipeline(handle, *shader_data);
        return pipeline_t;
    } else {
        // 打印一个 entity name 没有 VKR_shader
    }
    return VK_NULL_HANDLE;
}

VkPipelineLayout get_pipeline_layout(const entt::entity entity) {
    auto &handle                     = VK_backend::get();
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    if (const auto shader_temp = g_entt().try_get<VKR_shader_paths>(entity)) {
        if (!g_entt().all_of<std::shared_ptr<vk_shader_data> >(entity)) {
            g_entt().emplace<std::shared_ptr<vk_shader_data> >(entity, VKR_shader_init(*shader_temp));
        }
    }
    if (auto shader_data = g_entt().try_get<std::shared_ptr<vk_shader_data> >(entity)) {
        pipeline_layout = (*shader_data)->pipeline_layout;
        return pipeline_layout;
    } else {
        // 打印一个 entity name 没有 VKR_shader
    }
    return VK_NULL_HANDLE;
}


void global_uniform_buffer_update_function() {
    // 就是检查一下，已经给过 渲染线程，就添加一个 lambda 更新部分内容就好
    // global 相关的内容尽量只能偏移，
    const auto view = g_entt().view<global_uniform_buffer_update>();
    for (const auto &it: view) {
        update_global_bindings_to_descriptor_sets(it);
        g_entt().emplace_or_replace<descriptor_set_update>(it);
        auto lambda = [](const entt::entity entity) {
            if (g_entt().all_of<Scene_Component>(entity))
                g_entt().emplace_or_replace<descriptor_set_update>(entity);
        };
        add_recursion_function_to_children(it, lambda);
        g_entt().remove<global_uniform_buffer_update>(it);
    }
}

void uniform_buffer_update_function() {
    const auto view = g_entt().view<uniform_buffer_update>();
    for (const auto &it: view) {
        update_object_bindings_to_descriptor_sets(it);
        g_entt().emplace_or_replace<descriptor_set_update>(it);
        g_entt().remove<uniform_buffer_update>(it);
    }
}


void descriptor_set_update_function() {
    const auto view = g_entt().view<descriptor_set_update>();
    // 位置发生了更新，需要讲更新传递出去
    for (const auto it: view) {
        auto temp_des = get_descriptor_sets(it);

        auto lambda = [temp_des](const std::shared_ptr<VKR_object_proxy> &proxy) {
            proxy->vk_descriptor_set = temp_des;
        };
        update_VKR_object_proxy(it, lambda);
        g_entt().remove<descriptor_set_update>(it);
    }
}
