//
// Created by 潘鑫 on 2026/1/29.
//

#ifndef HELLO_MAC_BACKEND_H
#define HELLO_MAC_BACKEND_H

#include "create_pipeline.h"
#include "create_shader.h"
#include "descriptor.h"
#include "descriptor_organized_sets_and_bindings.h"
#include "logic_render_data.h"
#include "pipeline_layout.h"
#include "sets_and_bindings_layout.h"
#include "vertex_and_buffer_index.h"
#include "vk_render_to_image.h"
#include "vulkan_device_handle.h"
#include "vulkan_render_manage.h"


#include "update_push_constants_data.h"

void render_thread_start(VK_handle &handle);

void render_thread_stop();

void render_thread_stop_and_wait();

ShaderData get_shader_data();


inline bool Shader_paths::init() {
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


template<typename T1>
bool add_uniform_buffer_data(logic_render_data &logic_data, const std::string &binding_name, T1 binding_data) {
    for (auto const &[set_value, bindings_map]:
         logic_data.shader_paths_.shader_data_handle->model_sets_bindings) {
        for (const auto &[binding_value, info]: bindings_map) {
            if (info.binding_name == binding_name && info.resource_type == "uniform buffer") {
                auto [vk_buffer,offset]             = update_push_constants_data(binding_data); // 这里是一个需要同步的点
                Update_descriptor_binding temp      = {};
                temp.binding_name                   = binding_name;
                temp.resource_type                  = info.resource_type;
                temp.dstSet                         = set_value;
                temp.descriptor_write_binding.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                // temp.descriptor_write_bindings.dstSet           = descriptor_sets[0];
                temp.descriptor_write_binding.dstBinding       = 0;
                temp.descriptor_write_binding.dstArrayElement  = 0;
                temp.descriptor_write_binding.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                temp.descriptor_write_binding.descriptorCount  = 1;
                temp.descriptor_write_binding.pBufferInfo      = nullptr;
                temp.descriptor_write_binding.pImageInfo       = nullptr;
                temp.descriptor_write_binding.pTexelBufferView = nullptr;
                temp.bufferInfo                                = {true, {vk_buffer, offset, sizeof(binding_data)}};
                logic_data.update_descriptor_sets.emplace_back(temp);
                return true;
            }
        }
    }
    return false;
}


inline void update_bindings_to_descriptor_sets(logic_render_data &logic_data,
                                               const std::vector<VkDescriptorSet> &descriptor_sets) {
    // 以 binding 为一个最小数量
    auto &handle = VK_handle::get();
    std::vector<VkWriteDescriptorSet> descriptor_write_bindings{};
    descriptor_write_bindings.resize(logic_data.update_descriptor_sets.size());
    for (size_t i = 0; i < logic_data.update_descriptor_sets.size(); i++) {
        auto &binding_update                = logic_data.update_descriptor_sets[i];
        descriptor_write_bindings[i]        = binding_update.descriptor_write_binding;
        descriptor_write_bindings[i].dstSet = descriptor_sets[binding_update.dstSet];
        if (binding_update.bufferInfo.first) {
            descriptor_write_bindings[i].pBufferInfo = &binding_update.bufferInfo.second;
        } else if (binding_update.imageInfo.first) {
            descriptor_write_bindings[i].pImageInfo = &binding_update.imageInfo.second;;
        } else if (binding_update.TexelBufferView.first) {
            descriptor_write_bindings[i].pTexelBufferView = &binding_update.TexelBufferView.second;
        }
    }
    vkUpdateDescriptorSets(handle.device_,
                           static_cast<uint32_t>(descriptor_write_bindings.size()),
                           descriptor_write_bindings.data(),
                           0,
                           nullptr);
}


inline auto allocate_descriptor_sets(logic_render_data &logic_data) {
    // 这里就全部都是 渲染 某个物体时会 变更的数据了
    // 需要根据是全局还是物体单独的来进行创建了，全局的就获取全局的 descriptor_sets , 然后
    std::vector<VkDescriptorSet> descriptor_sets; // 这里是需要按照顺序的
    auto &handle = VK_handle::get();

    std::vector<VkDescriptorSet> global_descriptor_set;
    std::vector<VkDescriptorSet> object_descriptor_sets;

    auto &global_bindings_set = logic_data.shader_paths_.shader_data_handle->global_bindings_set;
    if (!global_bindings_set.empty()) {
        create_textures_to_gpu(handle, handle.get_command_pool());
        auto sets_flags = create_descriptor_sets_flags(handle,
                                                       global_bindings_set);
        global_descriptor_set = allocate_descriptor_sets(handle,
                                                         logic_data.shader_paths_.shader_data_handle->
                                                         global_descriptor_sets_layout,
                                                         &sets_flags);
        update_descriptor_sets(handle, handle.get_bindless_textures(), global_descriptor_set);
        // 更新应该被拆出来， 放到需要的位置再上传
    } {
        // 下面这段有问题，logic_data->shader_paths_.shader_data_handle->descriptor_sets_layout
        // 这个参数没有分离出来
        object_descriptor_sets = allocate_descriptor_sets(handle,
                                                          logic_data.shader_paths_.shader_data_handle->
                                                          model_descriptor_sets_layout,
                                                          nullptr);
    }
    descriptor_sets.reserve(global_descriptor_set.size() + object_descriptor_sets.size());

    descriptor_sets.insert(descriptor_sets.end(), global_descriptor_set.begin(), global_descriptor_set.end());
    descriptor_sets.insert(descriptor_sets.end(), object_descriptor_sets.begin(), object_descriptor_sets.end());
    return descriptor_sets;
}

inline bool add_object_to_render(logic_render_data &logic_data, entt::entity entity) {
    auto &handle    = VK_handle::get();
    auto pipeline_t = find_pipeline(handle, *logic_data.shader_paths_.shader_data_handle,
                                    VK_handle::get().get_pipeline_map());

    auto descriptor_sets = allocate_descriptor_sets(logic_data);
    update_bindings_to_descriptor_sets(logic_data, descriptor_sets);

    auto mesh = create_mesh(handle, logic_data, VK_handle::get().get_mesh_map());

    g_entt().emplace<std::shared_ptr<draw_need_vk> >(entity, std::make_shared<draw_need_vk>());
    auto &vk_data = g_entt().get<std::shared_ptr<draw_need_vk> >(entity);

    vk_data->mesh                   = mesh;
    vk_data->pipeline_layout        = logic_data.shader_paths_.shader_data_handle->pipeline_layout;
    vk_data->scissor                = VK_handle::get().get_scissor();
    vk_data->viewport               = VK_handle::get().get_viewport();
    vk_data->vk_pipeline            = pipeline_t;
    vk_data->debug_name             = logic_data.debug_name;
    vk_data->vk_descriptor_set      = descriptor_sets;
    vk_data->push_constants_address = 0;
    vk_render_queue::instance().render_object_need_init(vk_data);
    return true;
}

inline bool update_object_to_render(std::shared_ptr<draw_need_vk> render_object,
                                    const std::function<void(std::shared_ptr<draw_need_vk> render_object)> &callback) {
    vk_render_queue::instance().render_update(render_object, callback);
    return true;
}

inline bool clean_object_to_render(const entt::entity entity) {
    if (auto render = g_entt().try_get<std::shared_ptr<draw_need_vk> >(entity)) {
        vk_render_queue::instance().render_object_need_clean(*render);
        return true;
    }
    return false;
}


#endif //HELLO_MAC_BACKEND_H
