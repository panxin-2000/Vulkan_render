//
// Created by 潘鑫 on 2026/8/22.
//

#include "object_line.h"

#include "B_spline_cureve.h"
#include "Geometry_component.h"
#include "Geometry_data.h"
#include "mesh_component.h"
#include "name_component.h"
#include "scene_component.h"
#include "select_component.h"


void update_curve(const entt::entity entity) {
}


object_2d &object_2d::add_B_spline_curve() {
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

    Eigen::Vector2f scale{2.0f / logical_w, 2.0f / logical_h};
    Eigen::Vector2f translate{-1.0f - 0 * scale[0], -1.0f - 0 * scale[1]};

    add_push_constant_parameter("uScale", scale);
    add_push_constant_parameter("uTranslate", translate);
}

void object_line(const std::string &name) {
}

object_2d &object_2d::add_bezier_curve() {
    return *this;
}
