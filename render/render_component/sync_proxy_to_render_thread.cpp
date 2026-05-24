//
// Created by 潘鑫 on 2026/3/8.
//


#include "transform_component.h"
#include "Rect_2D_component.h"
#include "shader_component.h"
#include "VKR_proxy_component.h"
#include "vulkan_render_manage.h"

void sync_render_data_to_render_thread() {
    // 应该不止更新 position，还有很多的都需要更新
    update_camera_transform();
    update_object_transform_function();
    bindless_uniform_sampler2D_update_function();
    global_uniform_buffer_update_function();
    uniform_buffer_update_function();
    descriptor_set_update_function();
    add_new_proxy_to_render();
}


bool clean_VKR_object_proxy(const entt::entity entity) {
    if (auto render = Logic_entt().try_get<Proxy_entity>(entity)) {
        const auto entity_temp = render->entity_;
        auto lambda            = [entity_temp]() {
            Render_entt().emplace_or_replace<Render_destroy_tag>(entity_temp);
        };
        vk_render_queue::instance().render_update_entt(lambda);
        return true;
    }
    return false;
}
