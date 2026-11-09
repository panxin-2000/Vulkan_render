//
// Created by 潘鑫 on 2026/4/9.
//

#include "bounding_box_2d.h"
#include "engine.h"
#include "mesh_component.h"
#include "name_component.h"
#include "render_component/Geometry_component.h"
#include "render_state.h"
#include "scene_component.h"
#include "UI/UI_line.h"
#include "UI_render_common.h"

void update_UI_button_render_para(const entt::entity entity);

void draw_UI_line_impl(const entt::entity entity) {
    auto proxy_entity = get_proxy_entity(entity);
    logic_update_proxy<Name_component>(entity);

    Logic_entt().remove<Geometry_data>(entity); // 这里是很重要的一项，需要清除之前的几何数据


    auto shader_path = VKR_shader_paths{
        "2D/line", "2D/line", "", "", VK_PRIMITIVE_TOPOLOGY_LINE_LIST
    };

    add_shader(entity, shader_path);


    update_UI_button_render_para(entity);


    auto path = Logic_entt().get<UI_line_data>(entity).path;

    auto bounding_box = get_parent_Bounding_Box(entity);


    const Eigen::Vector2f err{bounding_box.min_x(), bounding_box.min_y()};
    for (auto &point: path) {
        point += err;
    }


    add_path(entity, path, {});

    logic_update_proxy(entity, get_VKR_mesh(entity));
    logic_update_proxy(entity, create_primitives(entity));
    logic_update_add_tag<Tag_UI_line>(entity);
}

/**
 * 负责重新绘制，根据当前的参数和它的parent_entity 的参数来完成重新绘制
 * @param entity
 * @return
 */
bool draw_UI_line(const entt::entity entity) {
    if (Logic_entt().all_of<Tag_UI_line_data_dirty, Tag_UI_line_renderable>(entity)) {
        draw_UI_line_impl(entity);
        logic_update_add_tag<Tag_UI_line>(entity);
        Logic_entt().remove<Tag_UI_line_data_dirty>(entity);
        return true;
    }
    return false;
}

bool update_UI_line(const entt::entity entity) {
    if (Logic_entt().all_of<Tag_UI_line>(entity)) {
        Logic_entt().emplace_or_replace<Tag_UI_line_data_dirty>(entity);
        return true;
    }
    return false;
}
