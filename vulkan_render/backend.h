//
// Created by 潘鑫 on 2026/1/29.
//

#ifndef HELLO_MAC_BACKEND_H
#define HELLO_MAC_BACKEND_H

#include "create_pipeline.h"
#include "descriptor_organized_sets_and_bindings.h"
#include "Geometry_data.h"
#include "name_component.h"
#include "vk_render_to_image.h"
#include "vulkan_device_handle.h"
#include "vulkan_render_manage.h"
#include "shader_component.h"

void render_thread_start(VK_handle &handle);

void render_thread_stop();

void render_thread_stop_and_wait();


inline bool add_object_to_render(const entt::entity entity) {
    auto &handle                     = VK_handle::get();
    VkPipeline pipeline_t            = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    if (const auto shader_temp = g_entt().try_get<VKR_shader>(entity)) {
        pipeline_t      = find_pipeline(handle, *shader_temp->shader_data_handle);
        pipeline_layout = shader_temp->shader_data_handle->pipeline_layout;
    } else {
        // 打印一个 entity name 没有 VKR_shader
    }

    //
    const auto descriptor_sets = allocate_descriptor_sets(entity);
    update_bindings_to_descriptor_sets(entity, descriptor_sets);

    const auto mesh = create_mesh(entity);
    if (!mesh.has_value()) {
        // 打印一个 entity name 读取 mesh 错误
    }

    g_entt().emplace<std::shared_ptr<draw_need_vk> >(entity, std::make_shared<draw_need_vk>());
    const auto &vk_data = g_entt().get<std::shared_ptr<draw_need_vk> >(entity);

    vk_data->mesh                   = mesh.value();
    vk_data->pipeline_layout        = pipeline_layout;
    vk_data->scissor                = VK_handle::get().get_scissor();
    vk_data->viewport               = VK_handle::get().get_viewport();
    vk_data->vk_pipeline            = pipeline_t;
    vk_data->debug_name             = get_entity_name(entity);
    vk_data->vk_descriptor_set      = descriptor_sets; // 唯一有可能每帧更新的部分
    vk_data->push_constants_address = 0;
    vk_render_queue::instance().render_object_need_init(vk_data);
    return true;
}

inline bool update_object_to_render(const std::shared_ptr<draw_need_vk> &render_object,
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
