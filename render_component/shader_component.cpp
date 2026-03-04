//
// Created by 潘鑫 on 2026/3/2.
//

#include "shader_component.h"
#include <memory_resource>

#include "create_shader.h"
#include "descriptor.h"
#include "pipeline_layout.h"
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


std::vector<VkDescriptorSet> allocate_descriptor_sets(const entt::entity entity) {
    // 这里就全部都是 渲染 某个物体时会 变更的数据了
    // 需要根据是全局还是物体单独的来进行创建了，全局的就获取全局的 descriptor_sets , 然后
    std::vector<VkDescriptorSet> descriptor_sets; // 这里是需要按照顺序的
    auto &handle = VK_handle::get();
    if (const auto shader_temp = g_entt().try_get<VKR_shader>(entity)) {
        std::vector<VkDescriptorSet> global_descriptor_set;
        std::vector<VkDescriptorSet> object_descriptor_sets;

        auto &global_bindings_set = shader_temp->shader_data_handle->global_bindings_set;
        if (!global_bindings_set.empty()) {
            create_textures_to_gpu(handle, handle.engine_.get_command_pool());
            auto sets_flags = create_descriptor_sets_flags(handle,
                                                           global_bindings_set);
            global_descriptor_set = allocate_descriptor_sets(handle,
                                                             shader_temp->shader_data_handle->
                                                             global_descriptor_sets_layout,
                                                             &sets_flags);
            update_descriptor_sets(handle, handle.get_bindless_textures(), global_descriptor_set);
            // 更新应该被拆出来， 放到需要的位置再上传
        } {
            // 下面这段有问题，logic_data->shader_paths_.shader_data_handle->descriptor_sets_layout
            // 这个参数没有分离出来
            object_descriptor_sets = allocate_descriptor_sets(handle,
                                                              shader_temp->shader_data_handle->
                                                              model_descriptor_sets_layout,
                                                              nullptr);
        }
        descriptor_sets.reserve(global_descriptor_set.size() + object_descriptor_sets.size());

        descriptor_sets.insert(descriptor_sets.end(), global_descriptor_set.begin(), global_descriptor_set.end());
        descriptor_sets.insert(descriptor_sets.end(), object_descriptor_sets.begin(), object_descriptor_sets.end());
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
