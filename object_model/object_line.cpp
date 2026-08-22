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


object_2d &object_2d::add_B_spline_curve() {
    auto &BSpline = Logic_entt().emplace<B_spline<Eigen::Vector2f> >(entity);
    BSpline.add_point({200, 200});
    BSpline.add_point({200, 600});
    BSpline.add_point({600, 200});
    BSpline.add_point({600, 600});
    BSpline.add_point({700, 700});
    const auto path = BSpline.get_path(1.25);

    add_path(entity, path, {});
    logic_update_proxy(entity, get_VKR_mesh(entity));
    logic_update_proxy(entity, create_primitives(entity));
    logic_update_add_tag<Line_tag>(entity);
    return *this;
}

object_line::object_line(const std::string &name) : object_2d(name) {
    add_shader_path(VKR_shader_paths{
                        "2D/line", "2D/line",
                        "", "", VK_PRIMITIVE_TOPOLOGY_LINE_LIST
                    });
    int logical_w, logical_h;
    const auto &backend = VK_backend::instance();

    SDL_GetWindowSize(backend.get_window(), &logical_w, &logical_h);

    float scale[2];
    scale[0] = 2.0f / logical_w; // Scale
    scale[1] = 2.0f / logical_h;
    float translate[2];
    translate[0] = -1.0f - 0 * scale[0]; // Translate
    translate[1] = -1.0f - 0 * scale[1];

    add_push_constant_parameter("uScale", scale);
    add_push_constant_parameter("uTranslate", translate);
}

void object_line(const std::string &name) {
}

object_2d &object_2d::add_bezier_curve() {
    Bezier<Eigen::Vector2f> bezier({200, 200}, {200, 600}, {600, 200}, {600, 600}, 1.25);
    std::vector<Eigen::Vector2f> path;
    bezier.Casteljau(&path);
    add_path(entity, path, {});
    logic_update_proxy(entity, get_VKR_mesh(entity));
    logic_update_proxy(entity, create_primitives(entity));
    logic_update_add_tag<Line_tag>(entity);
    return *this;
}
