//
// Created by 潘鑫 on 2026/3/6.
//

#include "VKR_proxy_component.h"
#include "create_pipeline.h"
#include "name_component.h"
#include "Rect_2D_component.h"
#include "vulkan_render_manage.h"
#include "pipeline_component.h"
#include "pipeline_layout_component.h"


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


entt::entity get_proxy_entity(const entt::entity logic_entity) {
    if (Logic_entt().all_of<Proxy_entity>(logic_entity)) {
        const auto vk_data     = Logic_entt().get<Proxy_entity>(logic_entity);
        const auto entity_temp = vk_data.entity_;
        return entity_temp;
    } else {
        return entt::null;
    }
}


void logic_update_debug_name(const entt::entity logic_entity, const std::string &debug_name) {
    if (auto proxy_entity = get_proxy_entity(logic_entity); proxy_entity != entt::null) {
        auto lambda = [ proxy_entity, debug_name ]() {
            auto &name      = Render_entt().get_or_emplace<Proxy_debug_name>(proxy_entity);
            name.debug_name = debug_name;
        };
        vk_render_queue::instance().render_update_entt(lambda);
    };
}

void logic_update_pipeline(const entt::entity logic_entity, const VkPipeline vk_pipeline) {
    if (auto proxy_entity = get_proxy_entity(logic_entity); proxy_entity != entt::null) {
        auto lambda = [ proxy_entity, vk_pipeline ]() {
            auto &temp       = Render_entt().get_or_emplace<Proxy_pipeline>(proxy_entity);
            temp.vk_pipeline = vk_pipeline;
        };
        vk_render_queue::instance().render_update_entt(lambda);
    };
}

void logic_update_pipeline_layout(const entt::entity logic_entity, const VkPipelineLayout pipeline_layout) {
    if (auto proxy_entity = get_proxy_entity(logic_entity); proxy_entity != entt::null) {
        auto lambda = [ proxy_entity, pipeline_layout ]() {
            auto &temp           = Render_entt().get_or_emplace<Proxy_pipeline_layout>(proxy_entity);
            temp.pipeline_layout = pipeline_layout;
        };
        vk_render_queue::instance().render_update_entt(lambda);
    };
}

void logic_update_proxy_descriptor_sets(const entt::entity logic_entity,
                                        const std::vector<DescriptorSet_ptr> vk_descriptor_sets) {
    if (auto proxy_entity = get_proxy_entity(logic_entity); proxy_entity != entt::null) {
        auto lambda = [ proxy_entity, vk_descriptor_sets ]() {
            auto &temp              = Render_entt().get_or_emplace<Proxy_descriptor_sets>(proxy_entity);
            temp.vk_descriptor_sets = vk_descriptor_sets;
        };
        vk_render_queue::instance().render_update_entt(lambda);
    };
}

void logic_update_Mesh(const entt::entity logic_entity,
                       const std::vector<VKR_Primitive> mesh) {
    if (auto proxy_entity = get_proxy_entity(logic_entity); proxy_entity != entt::null) {
        auto lambda = [ proxy_entity, mesh ]() {
            auto &temp = Render_entt().get_or_emplace<Mesh>(proxy_entity);
            temp.mesh  = mesh;
        };
        vk_render_queue::instance().render_update_entt(lambda);
    };
}

void logic_update_scissor(const entt::entity logic_entity,
                          const VkRect2D scissor) {
    if (auto proxy_entity = get_proxy_entity(logic_entity); proxy_entity != entt::null) {
        auto lambda = [ proxy_entity, scissor ]() {
            auto &temp   = Render_entt().get_or_emplace<Scissor>(proxy_entity);
            temp.scissor = scissor;
        };
        vk_render_queue::instance().render_update_entt(lambda);
    };
}

void logic_update_Viewport(const entt::entity logic_entity,
                           const VkViewport viewport) {
    if (auto proxy_entity = get_proxy_entity(logic_entity); proxy_entity != entt::null) {
        auto lambda = [ proxy_entity, viewport ]() {
            auto &temp    = Render_entt().get_or_emplace<Viewport>(proxy_entity);
            temp.viewport = viewport;
        };
        vk_render_queue::instance().render_update_entt(lambda);
    };
}


void add_new_peoxy_to_render_function() {
    const auto view = Logic_entt().view<add_to_render_tag>();
    for (const auto &it: view) {
        auto name = get_entity_name(it);
        logic_update_debug_name(it, name);

        auto mesh = get_VKR_mesh(it);
        logic_update_Mesh(it, mesh);
        auto pipeline_layout = get_pipeline_layout(it);
        logic_update_pipeline_layout(it, pipeline_layout);


        auto scissor = VK_backend::get().get_scissor();
        logic_update_scissor(it, scissor);
        auto viewport = VK_backend::get().get_viewport();
        logic_update_Viewport(it, viewport);

        auto vk_pipeline = get_pipeline(it);
        logic_update_pipeline(it, vk_pipeline);

        auto vk_descriptor_set = get_descriptor_sets(it); // 唯一有可能每帧更新的部分
        logic_update_proxy_descriptor_sets(it, vk_descriptor_set);

        add_render_UI_2D_tag(it);

        Logic_entt().remove<add_to_render_tag>(it);
    }
    vk_render_queue::instance().logic_add_finished();
}
