//
// Created by 潘鑫 on 2026/3/6.
//

#include "VKR_proxy_component.h"
#include "create_pipeline.h"
#include "name_component.h"
#include "vulkan_render_manage.h"


bool create_VKR_object_proxy(const entt::entity entity) {
    g_entt().remove<need_render_tag>(entity);

    auto &handle                     = VK_handle::get();
    VkPipeline pipeline_t            = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    if (auto shader_data = g_entt().try_get<std::shared_ptr<vk_shader_data> >(entity)) {
        pipeline_t      = find_pipeline(handle, *shader_data);
        pipeline_layout = (*shader_data)->pipeline_layout;
    } else {
        // 打印一个 entity name 没有 VKR_shader
    }
    const auto mesh = create_mesh(entity);
    if (!mesh.has_value()) {
        // 打印一个 entity name 读取 mesh 错误
    }

    g_entt().emplace<std::shared_ptr<VKR_object_proxy> >(entity, std::make_shared<VKR_object_proxy>());
    const auto &vk_data = g_entt().get<std::shared_ptr<VKR_object_proxy> >(entity);

    vk_data->mesh                   = mesh.value();
    vk_data->pipeline_layout        = pipeline_layout;
    vk_data->scissor                = VK_handle::get().get_scissor();
    vk_data->viewport               = VK_handle::get().get_viewport();
    vk_data->vk_pipeline            = pipeline_t;
    vk_data->debug_name             = get_entity_name(entity);
    vk_data->vk_descriptor_set      = get_descriptor_sets(entity); // 唯一有可能每帧更新的部分
    vk_data->push_constants_address = 0;
    vk_render_queue::instance().render_object_need_init(vk_data);
    return true;
}


bool update_VKR_object_proxy(const entt::entity entity, proxy_update_lambda callback) {
    if (auto proxy = g_entt().try_get<std::shared_ptr<VKR_object_proxy> >(entity)) {
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
