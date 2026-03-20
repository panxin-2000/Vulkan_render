//
// Created by 潘鑫 on 2026/3/18.
//

#include "3d_model_display.h"

#include "input_component.h"
#include "base_event.h"
#include "name_component.h"
#include "mesh_component.h"
#include "shader_component.h"
#include <Eigen/Eigen>
#include "base_geometry/intersect_function.h"
#include "model_transform_component.h"


entt::entity object_3d_model(const std::string &name, const std::string &mesh_path, const Point_3 offset,
                             const Eigen::Quaternionf &rotate) {
    const entt::entity entity = Logic_entt().create();
    Logic_entt().emplace<Name_component>(entity, name);
    Logic_entt().emplace<Input_Component>(entity, model_3d_Event);


    Logic_entt().emplace<VKR_shader_paths>(entity,
                                           "/Users/panxin/CLionProjects/hello_mac/render/shader/Phong.vert.spv",
                                           "/Users/panxin/CLionProjects/hello_mac/render/shader/Blinn_Phong.frag.spv",
                                           "", "");
    add_geometry_data(entity, mesh_path);
    auto [vertices, indices] = load_model(mesh_path);
    add_geometry_data(entity, vertices, indices);
    auto [min, max] = find_min_max_point(vertices);
    auto &AABB      = Logic_entt().get_or_emplace<AABB_centroid<Point_3> >(entity, AABB_centroid<Point_3>(min, max));

    // 更新物体的模型矩阵
    Logic_entt().emplace<model_transform>(entity, offset, rotate);
    auto &transform = Logic_entt().get<model_transform>(entity);

    const auto modelMatrix = transform.update_model_matrix();
    set_render_parameter(entity, "model_4x4", modelMatrix);

    world_root_add_child(entity);

    Logic_entt().emplace_or_replace<add_to_render_tag>(entity);
    return entity;
}


entt::entity add_sky_box(const std::string &name) {
    const entt::entity entity = Logic_entt().create();
    Logic_entt().emplace<Name_component>(entity, name);
    Logic_entt().emplace<Input_Component>(entity, model_3d_Event);


    Logic_entt().emplace<VKR_shader_paths>(entity,
                                           "/Users/panxin/CLionProjects/hello_mac/render/shader/skybox.vert.spv",
                                           "/Users/panxin/CLionProjects/hello_mac/render/shader/skybox.frag.spv",
                                           "", "");
    add_sky_box_data(entity);

    // 更新物体的模型矩阵

    world_root_add_child(entity);

    Logic_entt().emplace_or_replace<add_to_render_tag>(entity);

    return entity;
}
