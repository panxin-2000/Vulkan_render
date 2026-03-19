//
// Created by 潘鑫 on 2026/3/6.
//

#include "VKR_proxy_component.h"
#include "create_pipeline.h"
#include "name_component.h"
#include "Rect_2D_component.h"
#include "vulkan_render_manage.h"


void add_render_UI_2D_tag(entt::entity entity) {
    if (Logic_entt().all_of<UI_2D_tag, Proxy_entity>(entity)) {
        const auto &vk_data    = Logic_entt().get<Proxy_entity>(entity);
        const auto entity_temp = vk_data.entity_;
        auto lambda            = [entity_temp]() {
            Render_entt().emplace_or_replace<UI_2D_tag>(entity_temp);
        };
        vk_render_queue::instance().render_update_entt(lambda);
    }
}

void add_new_peoxy_to_render_function() {
    const auto view = Logic_entt().view<add_to_render_tag>(entt::exclude<Proxy_entity>);
    for (const auto &it: view) {
        const auto &vk_data    = Logic_entt().emplace<Proxy_entity>(it, Render_entt().create());
        const auto entity_temp = vk_data.entity_;

        auto name              = get_entity_name(it);
        auto mesh              = get_VKR_mesh(it);
        auto pipeline_layout   = get_pipeline_layout(it);
        auto scissor           = VK_backend::get().get_scissor();
        auto viewport          = VK_backend::get().get_viewport();
        auto vk_pipeline       = get_pipeline(it);
        auto vk_descriptor_set = get_descriptor_sets(it); // 唯一有可能每帧更新的部分

        auto lambda = [ entity_temp, name ,mesh, pipeline_layout ,scissor,viewport, vk_pipeline, vk_descriptor_set ]() {
            auto &proxy      = Render_entt().get_or_emplace<VKR_object_proxy>(entity_temp);
            proxy.debug_name = name;
            if (name.find("deferred_pass") != std::string::npos) {
                Render_entt().get_or_emplace<deferred_pass_tag>(entity_temp);
            }
            if (name.find("skybox") != std::string::npos) {
                Render_entt().get_or_emplace<skybox_tag>(entity_temp);
            }
            proxy.mesh               = mesh;
            proxy.pipeline_layout    = pipeline_layout;
            proxy.scissor            = scissor;
            proxy.viewport           = viewport;
            proxy.vk_pipeline        = vk_pipeline;
            proxy.vk_descriptor_sets = vk_descriptor_set;
        };

        vk_render_queue::instance().render_update_entt(lambda);

        add_render_UI_2D_tag(it);


        Logic_entt().remove<add_to_render_tag>(it);
    }
}
