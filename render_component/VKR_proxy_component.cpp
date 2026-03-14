//
// Created by 潘鑫 on 2026/3/6.
//

#include "VKR_proxy_component.h"
#include "create_pipeline.h"
#include "name_component.h"
#include "Rect_2D_component.h"
#include "vulkan_render_manage.h"


bool create_VKR_object_proxy(const entt::entity entity) {
    const auto &vk_data =
            g_entt().get_or_emplace<std::shared_ptr<VKR_object_proxy> >(entity, std::make_shared<VKR_object_proxy>());

    auto name = get_entity_name(entity);
    if (name.find("deferred_pass") != std::string::npos) {
        vk_data->pass_name = "deferred_pass";
    }
    vk_data->mesh                   = get_VKR_mesh(entity);
    vk_data->pipeline_layout        = get_pipeline_layout(entity);
    vk_data->scissor                = VK_backend::get().get_scissor();
    vk_data->viewport               = VK_backend::get().get_viewport();
    vk_data->vk_pipeline            = get_pipeline(entity);
    vk_data->debug_name             = get_entity_name(entity);
    vk_data->vk_descriptor_set      = get_descriptor_sets(entity); // 唯一有可能每帧更新的部分
    vk_render_queue::instance().render_object_need_init(vk_data);
    g_entt().remove<add_to_render_tag>(entity);

    return true;
}


bool update_VKR_object_proxy(const entt::entity entity, proxy_update_lambda callback) {
    if (const auto proxy = g_entt().try_get<std::shared_ptr<VKR_object_proxy> >(entity)) {
        vk_render_queue::instance().render_update(*proxy, callback);
        return true;
    }
    return false;
}

bool clean_VKR_object_proxy(const entt::entity entity) {
    if (auto render = g_entt().try_get<std::shared_ptr<VKR_object_proxy> >(entity)) {
        vk_render_queue::instance().render_object_need_clean(*render);
        return true;
    }
    return false;
}

void add_new_peoxy_to_render_function() {
    const auto view = g_entt().view<add_to_render_tag>(entt::exclude<std::shared_ptr<VKR_object_proxy> >);
    for (const auto &it: view) {
        create_VKR_object_proxy(it); // 因为这里没有区分。全部都在场景的根节点之下
    }
}
