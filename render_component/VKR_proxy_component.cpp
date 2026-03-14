//
// Created by 潘鑫 on 2026/3/6.
//

#include "VKR_proxy_component.h"
#include "create_pipeline.h"
#include "name_component.h"
#include "Rect_2D_component.h"
#include "vulkan_render_manage.h"


bool clean_VKR_object_proxy(const entt::entity entity) {
    if (auto render = LGC_entt().try_get<RND_entity>(entity)) {
        auto lambda = [render]() { RND_entt().emplace_or_replace<RND_destroy_tag>(render->entity_); };
        vk_render_queue::instance().render_update_entt(*render, lambda);
        return true;
    }
    return false;
}

void add_new_peoxy_to_render_function() {
    const auto view = LGC_entt().view<add_to_render_tag>(entt::exclude<RND_entity>);
    for (const auto &it: view) {
        const auto &vk_data = LGC_entt().emplace<RND_entity>(it, RND_entt().create());

        auto name              = get_entity_name(it);
        auto mesh              = get_VKR_mesh(it);
        auto pipeline_layout   = get_pipeline_layout(it);
        auto scissor           = VK_backend::get().get_scissor();
        auto viewport          = VK_backend::get().get_viewport();
        auto vk_pipeline       = get_pipeline(it);
        auto vk_descriptor_set = get_descriptor_sets(it); // 唯一有可能每帧更新的部分

        auto lambda = [ vk_data, name ,mesh, pipeline_layout ,scissor,viewport, vk_pipeline, vk_descriptor_set ]() {
            auto &proxy      = RND_entt().get_or_emplace<VKR_object_proxy>(vk_data.entity_);
            proxy.debug_name = name;
            if (name.find("deferred_pass") != std::string::npos) {
                RND_entt().get_or_emplace<deferred_pass_tag>(vk_data.entity_);
            }
            proxy.mesh              = mesh;
            proxy.pipeline_layout   = pipeline_layout;
            proxy.scissor           = scissor;
            proxy.viewport          = viewport;
            proxy.vk_pipeline       = vk_pipeline;
            proxy.vk_descriptor_set = vk_descriptor_set;
        };

        vk_render_queue::instance().render_update_entt(vk_data, lambda);
        LGC_entt().remove<add_to_render_tag>(it);
    }
}
