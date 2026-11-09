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

void update_UI_button_render_para(const entt::entity entity);

void draw_UI_line_impl(const entt::entity entity) {
    auto proxy_entity = get_proxy_entity(entity);
    logic_update_proxy<Name_component>(entity);

    Logic_entt().remove<Geometry_data>(entity); // 这里是很重要的一项，需要清除之前的几何数据


    auto shader_path = VKR_shader_paths{
        "2D/line", "2D/line", "", "", VK_PRIMITIVE_TOPOLOGY_LINE_LIST
    };
    if (!Logic_entt().all_of<VKR_shader_paths>(entity)) {
        Logic_entt().emplace_or_replace<VKR_shader_paths>(entity, shader_path);
        logic_update_proxy<VKR_shader_paths>(entity);
    }
    if (!Logic_entt().all_of<Shader_data>(entity)) {
        auto shader_data = Engine::instance().get_shader_manager().find(shader_path);
        Logic_entt().emplace_or_replace<Shader_data>(entity, shader_data);
        logic_update_proxy<Shader_data>(entity);
    }

    update_UI_button_render_para(entity);


    auto path = Logic_entt().get<UI_line_data>(entity).path;


    float min_x = 0;
    float min_y = 0;


    if (auto parent = get_parent(entity); parent != entt::null) {
        if (Logic_entt().all_of<Bounding_Box_2D>(parent)) {
            const auto &bounding = Logic_entt().get<Bounding_Box_2D>(parent);
            min_x                = bounding.box[0];
            min_y                = bounding.box[1];
        }
    }
    Eigen::Vector2f err{min_x, min_y};
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
