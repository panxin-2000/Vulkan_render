//
// Created by 潘鑫 on 2026/3/6.
//

#include "VKR_proxy_component.h"
#include "create_pipeline.h"
#include "name_component.h"
#include "Rect_2D_component.h"
#include "vulkan_render_manage.h"


bool create_VKR_object_proxy(const entt::entity entity) {
    const auto view = LGC_entt().view<add_to_render_tag>(entt::exclude<std::shared_ptr<VKR_object_proxy> >);
    for (const auto &it: view) {
        const auto &vk_data =
                LGC_entt().emplace<RND_entity>(entity, RND_entt().create());

        auto name = get_entity_name(entity);

        auto lambda = [ name]() {
            // render_object.debug_name = name;
        };

        entt_update_VKR_object_proxy(vk_data, lambda);
        LGC_entt().remove<add_to_render_tag>(entity);
    }
    return true;
}


bool entt_update_VKR_object_proxy(const RND_entity entity, entt_proxy_update_lambda callback) {
    vk_render_queue::instance().render_update_entt(entity, callback);
    return false;
}


bool update_VKR_object_proxy(const entt::entity entity, proxy_update_lambda callback) {
    if (const auto proxy = LGC_entt().try_get<std::shared_ptr<VKR_object_proxy> >(entity)) {
        vk_render_queue::instance().render_update(*proxy, callback);
        return true;
    }
    return false;
}

bool clean_VKR_object_proxy(const entt::entity entity) {
    if (auto render = LGC_entt().try_get<std::shared_ptr<VKR_object_proxy> >(entity)) {
        vk_render_queue::instance().render_object_need_clean(*render);
        return true;
    }
    return false;
}

void add_new_peoxy_to_render_function() {
    const auto view = LGC_entt().view<add_to_render_tag>(entt::exclude<std::shared_ptr<VKR_object_proxy> >);
    for (const auto &it: view) {
        const auto &vk_data =
                LGC_entt().get_or_emplace<std::shared_ptr<VKR_object_proxy> >(it, std::make_shared<VKR_object_proxy>());
        auto name = get_entity_name(it);
        if (name.find("deferred_pass") != std::string::npos) {
            vk_data->pass_name = "deferred_pass";
        }
        vk_data->mesh              = get_VKR_mesh(it);
        vk_data->pipeline_layout   = get_pipeline_layout(it);
        vk_data->scissor           = VK_backend::get().get_scissor();
        vk_data->viewport          = VK_backend::get().get_viewport();
        vk_data->vk_pipeline       = get_pipeline(it);
        vk_data->debug_name        = get_entity_name(it);
        vk_data->vk_descriptor_set = get_descriptor_sets(it); // 唯一有可能每帧更新的部分
        vk_render_queue::instance().render_object_need_init(vk_data);
        LGC_entt().remove<add_to_render_tag>(it);
    }
}
