//
// Created by 潘鑫 on 2026/3/8.
//


#include "model_transform_component.h"
#include "Rect_2D_component.h"
#include "shader_component.h"
#include "VKR_proxy_component.h"
#include "vulkan_render_manage.h"

void sync_render_data_to_render_thread() {
    // 应该不止更新 position，还有很多的都需要更新
    update_camera_transform();
    update_2D_UI_object_function();
    global_uniform_buffer_update_function();
    uniform_buffer_update_function();
    descriptor_set_update_function();
    add_new_peoxy_to_render_function();
}


bool clean_VKR_object_proxy(const entt::entity entity) {
    if (auto render = Logic_entt().try_get<Render_entity>(entity)) {
        auto lambda = [render]() { Render_entt().emplace_or_replace<Render_destroy_tag>(render->entity_); };
        vk_render_queue::instance().render_update_entt(*render, lambda);
        return true;
    }
    return false;
}
