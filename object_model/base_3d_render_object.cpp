//
// Created by 潘鑫 on 2026/3/18.
//

#include "base_3d_render_object.h"

#include "input_component.h"
#include "name_component.h"
#include "mesh_component.h"
#include "shader_component.h"
#include <Eigen/Eigen>
#include "transform_component.h"
#include <meshoptimizer.h>

#include "B_spline_cureve.h"
#include "parse_geometry_file.h"
#include "../render/render_common/render_state.h"
#include "world_scene_root.h"


object_3d::object_3d(const std::string &name) : logic_render_object(name) {
    logic_create_proxy(entity);
    add_model_3d_Event(entity);
    logic_update_proxy<Name_component>(entity);
    world_root_add_child(entity);
}

object_3d &object_3d::object_3d_add_mesh(const AABB_min_max<Point_3> &bounding_box) {
    add_box_data(entity, bounding_box);
    logic_update_proxy(entity, get_VKR_mesh(entity));
    logic_update_proxy(entity, create_primitives(entity));
    logic_update_add_tag<opacity_tag>(entity);
    return *this;
}

object_3d &object_3d::add_manifold_mesh(manifold::MeshGL &mesh) {
    const auto vertex_count = mesh.vertProperties.size() / mesh.numProp;
    // 索引（Indices）推荐“原地优化”，但顶点（Vertices）推荐“非原地优化（重新排列）”

    // 先优化顶点缓存 (减少 VS 计算)
    meshopt_optimizeVertexCache(mesh.triVerts.data(),
                                mesh.triVerts.data(),
                                mesh.triVerts.size(),
                                vertex_count);

    // 再优化过渡绘制 (减少 PS 浪费)
    // 通过重排三角形减少 Overdraw（适合深度前传或不透明物体）
    meshopt_optimizeOverdraw(mesh.triVerts.data(),
                             mesh.triVerts.data(),
                             mesh.triVerts.size(),
                             mesh.vertProperties.data(), vertex_count,
                             mesh.numProp * sizeof(float), 1.05f);

    // 最后排列顶点 (提升内存访问效率)
    // 重新排列顶点属性数据，使内存访问与索引顺序对齐
    std::vector<float> optimized_vertices(mesh.vertProperties.size());
    meshopt_optimizeVertexFetch(
                                optimized_vertices.data(),                  // 输出
                                mesh.triVerts.data(), mesh.triVerts.size(), // 已优化的索引
                                mesh.vertProperties.data(),                 // 原始顶点属性
                                vertex_count,                               // 顶点数
                                mesh.numProp * sizeof(float)                // 每个顶点的字节步长
                               );
    // 将优化后的顶点数据写回
    mesh.vertProperties = std::move(optimized_vertices);
    auto sp_vertices    = std::make_shared<std::vector<Vertex> >();
    auto sp_indices     = std::make_shared<std::vector<uint16_t> >();
    sp_vertices->resize(mesh.vertProperties.size() / mesh.numProp);
    sp_indices->reserve(mesh.triVerts.size());
    memcpy(sp_vertices->data(), mesh.vertProperties.data(), sp_vertices->size() * sizeof(Vertex));
    for (int i = 0; i < mesh.triVerts.size(); i++) {
        sp_indices->push_back(mesh.triVerts.at(i));
    }
    add_geometry_data(entity, sp_vertices, sp_indices);
    logic_update_proxy(entity, get_VKR_mesh(entity));
    logic_update_proxy(entity, create_primitives(entity));
    return *this;
}

object_3d &object_3d::add_mesh(const std::string &mesh_path) {
    auto aabb = load_model(entity, mesh_path);
    logic_update_proxy(entity, get_VKR_mesh(entity));
    const auto primitives = create_primitives(entity);
    logic_update_proxy(entity, primitives);
    return *this;
}

object_3d &object_3d::add_mesh(const AABB_min_max<Point_3> &bounding_box) {
    add_box_data(entity, bounding_box);
    logic_update_proxy(entity, get_VKR_mesh(entity));
    logic_update_proxy(entity, create_primitives(entity));
    logic_update_add_tag<opacity_tag>(entity);
    return *this;
}

object_3d &object_3d::add_postprocess() {
    logic_update_add_tag<compute_postprocess_tag>(entity);
    return *this;
}


object_3d &object_3d::add_sky_box() {
    logic_update_proxy<Name_component>(entity);
    add_shader(entity,
               "skybox",
               "skybox",
               "", "");
    add_box_data(entity);
    const auto mesh = get_VKR_mesh(entity);
    auto primitives = create_primitives(entity);
    std::vector<VKR_Render_state> render_states;
    render_states.resize(primitives.size());
    for (auto &render_state: render_states) {
        render_state.set_front_face(VK_FRONT_FACE_COUNTER_CLOCKWISE);
        render_state.set_VkCullModeFlags(VK_CULL_MODE_FRONT_BIT);
    }
    logic_update_proxy(entity, mesh);
    logic_update_proxy(entity, primitives);
    logic_update_proxy(entity, render_states);
    logic_update_add_tag<skybox_tag>(entity);
    return *this;
}

object_3d &object_3d::set_transform(const Eigen::Vector3f offset, const Eigen::Quaternionf &rotate) {
    auto matrix          = Logic_entt().emplace<Transform>(entity, offset, rotate);
    auto matrix_2        = matrix.get_transform_matrix();
    auto matrices_render = std::make_shared<std::vector<Transform_Matrix> >();
    matrices_render->push_back(static_cast<std::vector<Transform_Matrix>::value_type>(matrix_2));
    set_render_parameter(entity, "model_matrix_parameters", matrices_render);
    Logic_entt().emplace<Transform_matrix_dirty>(entity);

    return *this;
}
