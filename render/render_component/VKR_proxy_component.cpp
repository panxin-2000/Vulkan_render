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


void add_new_proxy_to_render() {
    const auto view = Logic_entt().view<add_to_render_tag>();
    for (const auto &it: view) {
        logic_update_proxy<Name_component>(it);

        const auto mesh = get_VKR_mesh(it);
        logic_update_proxy(it, mesh);
        const auto pipeline_layout = get_pipeline_layout(it);
        logic_update_proxy<VkPipelineLayout>(it, pipeline_layout);

        const auto scissor = VK_backend::get().get_scissor();
        logic_update_proxy(it, scissor);
        const auto viewport = VK_backend::get().get_viewport();
        logic_update_proxy(it, viewport);

        const auto vk_pipeline = get_pipeline(it);
        logic_update_proxy(it, vk_pipeline);

        const auto vk_descriptor_set = get_descriptor_sets(it); // 唯一有可能每帧更新的部分
        logic_update_proxy(it, vk_descriptor_set);

        Logic_entt().remove<add_to_render_tag>(it);
    }
    vk_render_queue::instance().logic_add_finished();
}
