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


void render_thread_start(VKDevice &handle);

void render_thread_stop();

void render_thread_stop_and_wait();


inline bool add_object_to_render(VKDevice &handle, logic_render_data *render_object) {
    //
    if (render_object != nullptr) {
        auto shaderStages = find_graphics_shader_module(handle, render_object->vertexPath_,
                                                        render_object->fragmentPath_,
                                                        render_object->geometryPath_);
        auto organized_sets_and_bindings =
                organize_graphics_descriptor_set_and_binding_layouts(render_object->vertexPath_,
                                                                     render_object->fragmentPath_,
                                                                     render_object->geometryPath_);
        auto shader_key = get_shader_key(render_object->vertexPath_,
                                         render_object->fragmentPath_,
                                         render_object->geometryPath_);


        const auto descriptor_sets_layout =
                create_descriptor_sets_layout(handle, shader_key, organized_sets_and_bindings);
        auto sets_flags      = create_descriptor_sets_flags(handle, organized_sets_and_bindings);
        auto pipeline_layout = create_pipeline_layout(handle, shader_key, descriptor_sets_layout);

        const auto vertexInputState = vertex_input_position_normal_uv();
        auto pipeline_t             = find_pipeline(handle, shader_key, pipeline_layout, shaderStages,
                                        VKDevice::get().get_pipeline_map());
        if (pipeline_t == VK_NULL_HANDLE) {
            // continue;
        }
        std::vector<VkDescriptorSet> descriptor_set_texture;
        if (render_object->debug_name == "blender Suzanne") {
            create_textures_to_gpu(handle, handle.get_command_pool());
            auto temp = allocate_descriptor_sets(handle, descriptor_sets_layout[0],
                                                 sets_flags[0]);
            // g_hjk = descriptor_set_texture;
            update_descriptor_sets(handle, handle.get_bindless_textures(), temp);
            // 更新应该被拆出来， 放到需要的位置再上传
            descriptor_set_texture.push_back(temp[0]);
        }
        update_shader_data(handle.engine_); // 这里是一个需要同步的点

        auto push_constants = handle.engine_.get_current_shader_data_buffer().deviceAddress;

        auto mesh                       = create_mesh(handle, render_object, VKDevice::get().get_mesh_map());
        auto vk_data                    = new draw_need_vk;
        vk_data->mesh                   = mesh;
        vk_data->pipeline_layout        = pipeline_layout;
        vk_data->scissor                = VKDevice::get().get_scissor();
        vk_data->viewport               = VKDevice::get().get_viewport();
        vk_data->vk_pipeline            = pipeline_t;
        vk_data->debug_name             = render_object->debug_name;
        vk_data->vk_descriptor_set      = descriptor_set_texture;
        vk_data->push_constants_address = push_constants;
        vk_render_queue::instance().render_object_need_init(vk_data);
        return true;
    }
    return false;
}

inline bool update_object_to_render(logic_render_data *render_object) {
    auto vk_data = new draw_need_vk;
    vk_render_queue::instance().render_object_need_init(vk_data);
    return true;
}

inline bool clean_object_to_render(logic_render_data *render_object) {
    auto vk_data = new draw_need_vk;
    vk_render_queue::instance().render_object_need_init(vk_data);
    return true;
}

#endif //HELLO_MAC_BACKEND_H
