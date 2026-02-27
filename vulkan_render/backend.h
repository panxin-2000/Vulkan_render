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
    if (data == nullptr) data = std::make_shared<vk_shader_data>();
    auto &handle                             = VK_handle::get();
    data->pipeline_shader_stage_create_infos = find_graphics_shader_module(handle, *this);
    data->organized_sets_and_bindings        = organize_graphics_descriptor_set_and_binding_layouts(*this);
    data->shader_key                         = get_shader_key(*this);
    // 下面这两个对于创建的顺序有点要求，上面的没有顺序要求
    data->descriptor_sets_layout =
            create_descriptor_sets_layout(handle, data->shader_key, data->organized_sets_and_bindings);
    data->pipeline_layout = create_pipeline_layout(handle, data->shader_key, data->descriptor_sets_layout);

    return true;
}


template<typename T1>
bool add_uniform_buffer_data(logic_render_data *logic_data, const std::string &binding_name, T1 binding_data) {
    uint32_t dstSet = 0;
    for (const auto &map: logic_data->shader_paths_.data->organized_sets_and_bindings) {
        for (const auto &[fst, snd]: map) {
            if (snd.binding_name == binding_name && snd.resource_type == "uniform buffer") {
                auto [vk_buffer,offset]             = update_push_constants_data(binding_data); // 这里是一个需要同步的点
                Update_descriptor_binding temp      = {};
                temp.binding_name                   = binding_name;
                temp.resource_type                  = snd.resource_type;
                temp.dstSet                         = dstSet;
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
                logic_data->update_descriptor_sets.emplace_back(temp);
                return true;
            }
        }
        dstSet++;
    }
    return false;
}


inline void update_bindings_to_descriptor_sets(logic_render_data *logic_data,
                                               const std::vector<VkDescriptorSet> &descriptor_sets) {
    // 以 binding 为一个最小数量
    auto &handle = VK_handle::get();
    std::vector<VkWriteDescriptorSet> descriptor_write_bindings{};
    descriptor_write_bindings.resize(logic_data->update_descriptor_sets.size());
    for (size_t i = 0; i < logic_data->update_descriptor_sets.size(); i++) {
        auto &binding_update                = logic_data->update_descriptor_sets[i];
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

inline bool add_object_to_render(logic_render_data *logic_data) {
    auto &handle = VK_handle::get();
    if (logic_data != nullptr) {
        logic_data->shader_paths_.init();
        auto pipeline_t = find_pipeline(handle, *logic_data->shader_paths_.data,
                                        VK_handle::get().get_pipeline_map());


        // 这里就全部都是 渲染 某个物体时会 变更的数据了
        // 需要根据是全局还是物体单独的来进行创建了，全局的就获取全局的 descriptor_sets , 然后
        std::vector<VkDescriptorSet> descriptor_sets;
        if (logic_data->debug_name == "blender Suzanne") {
            create_textures_to_gpu(handle, handle.get_command_pool());
            auto sets_flags = create_descriptor_sets_flags(handle,
                                                           logic_data->shader_paths_.data->organized_sets_and_bindings);
            descriptor_sets = allocate_descriptor_sets(handle, logic_data->shader_paths_.data->descriptor_sets_layout,
                                                       &sets_flags);
            update_descriptor_sets(handle, handle.get_bindless_textures(), descriptor_sets);
            // 更新应该被拆出来， 放到需要的位置再上传
        } else {
            descriptor_sets = allocate_descriptor_sets(handle, logic_data->shader_paths_.data->descriptor_sets_layout,
                                                       nullptr);
        }


        struct Shader_Data_po {
            matrix_4x4 projection;
            matrix_4x4 view;
            matrix_4x4 model;
        };
        Shader_Data_po temp;

        identity_matrix_4x4(&temp.projection);
        identity_matrix_4x4(&temp.view);
        UI_matrix_4x4(&temp.model, 1280, 720);

        add_uniform_buffer_data(logic_data, "UBO", temp);
        update_bindings_to_descriptor_sets(logic_data, descriptor_sets);


        auto mesh                       = create_mesh(handle, logic_data, VK_handle::get().get_mesh_map());
        auto vk_data                    = new draw_need_vk;
        logic_data->proxy               = vk_data;
        vk_data->mesh                   = mesh;
        vk_data->pipeline_layout        = logic_data->shader_paths_.data->pipeline_layout;
        vk_data->scissor                = VK_handle::get().get_scissor();
        vk_data->viewport               = VK_handle::get().get_viewport();
        vk_data->vk_pipeline            = pipeline_t;
        vk_data->debug_name             = logic_data->debug_name;
        vk_data->vk_descriptor_set      = descriptor_sets;
        vk_data->push_constants_address = 0;
        vk_render_queue::instance().render_object_need_init(vk_data);
        return true;
    }
    return false;
}

inline bool update_object_to_render(draw_need_vk *render_object,
                                    const std::function<void(draw_need_vk *render_object)> &callback) {
    vk_render_queue::instance().render_update(render_object, callback);
    return true;
}

inline bool clean_object_to_render(const logic_render_data *render_object) {
    if (render_object != nullptr && render_object->proxy != nullptr) {
        vk_render_queue::instance().render_object_need_clean(render_object->proxy);
        return true;
    }
    return false;
}


#endif //HELLO_MAC_BACKEND_H
