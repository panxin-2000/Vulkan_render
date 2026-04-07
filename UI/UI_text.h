//
// Created by 潘鑫 on 2026/4/8.
//

#ifndef HELLO_MAC_UI_TEXT_H
#define HELLO_MAC_UI_TEXT_H

#include "name_component.h"
#include "Rect_2D_component.h"


entt::entity UI_text(const std::string &name,
                     float min_x,
                     float min_y,
                     float max_x,
                     float max_y) {
    std::string_view df = "";

    LOG_INFO(g_log(), "UI create  {} {} {} {} {} ", name, min_x, min_y, max_x, max_y);

    const entt::entity entity = Logic_entt().create();
    Logic_entt().emplace<Proxy_entity>(entity, Render_entt().create());


    Logic_entt().emplace<Rect_2D_transform>(entity);
    Logic_entt().emplace<VKR_shader_paths>(entity,
                                           "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_MSDF_text.vert.spv",
                                           "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_MSDF_text.frag.spv",
                                           "", "");

    if (auto *scene_node = Logic_entt().try_get<Rect_2D_transform>(entity)) {
        scene_node->set_bounding_box({min_x, min_y}, {max_x, max_y});
    }
    Logic_entt().emplace<Drag_event>(entity);
    Logic_entt().emplace<Name_component>(entity, name);
    add_geometry_data(entity, {min_x, min_y, 0.0f}, {max_x, max_y, 0.0f});



    matrix_4x4 model;
    UI_matrix_4x4(&model, {1, 1}, {0, 0});
    set_render_parameter(entity, "model_4x4", model);

    Logic_entt().emplace_or_replace<add_to_render_tag>(entity);
    Logic_entt().emplace_or_replace<UI_2D_tag>(entity);

    scene_root_add_child(entity);
    return entity;

    // 还想需要添加位置的，以及缩放。缩放暂时不需要，需要添加层。
    /***************添加到渲染管理器**********************/
}


#endif //HELLO_MAC_UI_TEXT_H
