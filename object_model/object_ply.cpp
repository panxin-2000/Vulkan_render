//
// Created by 潘鑫 on 2026/8/22.
//

#include "object_ply.h"

#include "name_component.h"
#include "mesh_component.h"
#include "shader_component.h"
#include <Eigen/Eigen>
#include "transform_component.h"

#include "B_spline_cureve.h"
#include "../render/render_common/PBR_component.h"
#include "world_scene_root.h"
#include "../UI/PLYLoader.h"


entt::entity object_ply_model(const std::string &name, const std::string &file_path, const Eigen::Vector3f offset,
                              const Eigen::Quaternionf &rotate) {
    const entt::entity entity = Logic_entt().create();
    logic_create_proxy(entity);

    Logic_entt().emplace<Name_component>(entity, name);
    add_model_3d_Event(entity);

    // add_shader(entity,
    //            "",
    //            "",
    //            "",
    //            "");
    // 这里需要解决另一个问题，想要运行这个，需要不止一个 shader

    int _degree        = 0;
    auto _gaussianData = PLYLoader::LoadPLY(file_path, _degree);
    auto aabb          = AABB_min_max<Point_3>{{1, 1, 1}, {2, 2, 2}};
    auto &AABB         = Logic_entt().get_or_emplace<AABB_min_max<Point_3> >(entity, aabb);

    // 更新物体的模型矩阵
    auto transform = Logic_entt().emplace<Transform>(entity, offset, rotate);

    const auto modelMatrix = get_model_matrix(transform);
    set_render_parameter(entity, "model_4x4", modelMatrix);

    world_root_add_child(entity);
    auto material = Logic_entt().get_or_emplace<PBR_component>(entity);
    set_render_parameter(entity, "object_material", material);

    logic_update_proxy<Name_component>(entity);
    logic_update_proxy(entity, get_VKR_mesh(entity));
    logic_update_proxy(entity, create_primitives(entity));
    return entity;
}
