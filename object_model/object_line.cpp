//
// Created by 潘鑫 on 2026/8/22.
//

#include "object_line.h"

#include "B_spline_cureve.h"
#include "Geometry_data.h"
#include "mesh_component.h"
#include "name_component.h"
#include "scene_component.h"
#include "select_component.h"


void update_curve(const entt::entity entity) {
    if (auto BSpline = Logic_entt().try_get<B_spline<Eigen::Vector2f> >(entity)) {
        const auto path = BSpline->get_path(1.25);
        add_path(entity, path, {});
        logic_update_proxy(entity, get_VKR_mesh(entity));
    }
}


entt::entity object_line(const std::string &name) {
    const entt::entity entity = Logic_entt().create();
    logic_create_proxy(entity);
    Logic_entt().emplace<Name_component>(entity, name);
    add_model_3d_Event(entity);
    Logic_entt().emplace<select_component>(entity);

    Logic_entt().emplace<Shader_data>(entity, Engine::instance().get_shader_manager().get_line_shader_data());
    logic_update_proxy<Shader_data>(entity);

    auto &BSpline = Logic_entt().emplace<B_spline<Eigen::Vector2f> >(entity);
    BSpline.add_point({200, 200});
    BSpline.add_point({200, 600});
    BSpline.add_point({600, 200});
    BSpline.add_point({600, 600});
    BSpline.add_point({700, 700});
    const auto path = BSpline.get_path(1.25);

    add_path(entity, path, {});
    UI_root_add_child(entity);
    logic_update_proxy<Name_component>(entity);
    logic_update_proxy(entity, get_VKR_mesh(entity));
    logic_update_proxy(entity, create_primitives(entity));

    int logical_w, logical_h;
    const auto &backend = VK_backend::instance();

    SDL_GetWindowSize(backend.get_window(), &logical_w, &logical_h);

    float scale[2];
    scale[0] = 2.0f / logical_w; // Scale
    scale[1] = 2.0f / logical_h;
    float translate[2];
    translate[0] = -1.0f - 0 * scale[0]; // Translate
    translate[1] = -1.0f - 0 * scale[1];

    set_push_constant_parameter(entity, "uScale", scale);
    set_push_constant_parameter(entity, "uTranslate", translate);

    logic_update_add_tag<Line_tag>(entity);
    return entity;
}


entt::entity object_line_old(const std::string &name) {
    const entt::entity entity = Logic_entt().create();
    logic_create_proxy(entity);
    Logic_entt().emplace<Name_component>(entity, name);
    add_model_3d_Event(entity);
    Logic_entt().emplace<Shader_data>(entity, Engine::instance().get_shader_manager().get_line_shader_data());
    logic_update_proxy<Shader_data>(entity);
    // add_line(entity, {40, 40}, {600, 600});
    Bezier<Eigen::Vector2f> bezier({200, 200}, {200, 600}, {600, 200}, {600, 600}, 1.25);
    std::vector<Eigen::Vector2f> path;
    bezier.Casteljau(&path);
    add_path(entity, path, {});

    UI_root_add_child(entity);
    logic_update_proxy<Name_component>(entity);
    logic_update_proxy(entity, get_VKR_mesh(entity));
    logic_update_proxy(entity, create_primitives(entity));

    int logical_w, logical_h;
    const auto &backend = VK_backend::instance();

    SDL_GetWindowSize(backend.get_window(), &logical_w, &logical_h);

    float scale[2];
    scale[0] = 2.0f / logical_w; // Scale
    scale[1] = 2.0f / logical_h;
    float translate[2];
    translate[0] = -1.0f - 0 * scale[0]; // Translate
    translate[1] = -1.0f - 0 * scale[1];

    set_push_constant_parameter(entity, "uScale", scale);
    set_push_constant_parameter(entity, "uTranslate", translate);

    logic_update_add_tag<Line_tag>(entity);
    return entity;
}
