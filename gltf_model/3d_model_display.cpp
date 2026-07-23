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
#include "transform_component.h"
#include <meshoptimizer.h>

#include "parse_geometry_file.h"
#include "PBR_component.h"
#include "../UI/PLYLoader.h"


entt::entity object_ply_model(const std::string &name, const std::string &file_path, const Point_3 offset,
                              const Eigen::Quaternionf &rotate) {
    const entt::entity entity = Logic_entt().create();
    logic_create_proxy(entity);

    Logic_entt().emplace<Name_component>(entity, name);
    Logic_entt().emplace<Input_Component>(entity, model_3d_Event);


    add_shader(entity,
               "",
               "",
               "",
               "");
    // 这里需要解决另一个问题，想要运行这个，需要不止一个 shader

    int _degree        = 0;
    auto _gaussianData = PLYLoader::LoadPLY(file_path, _degree);
    auto aabb          = AABB_centroid<Point_3>{{1, 1, 1}, {2, 2, 2}};
    auto &AABB         = Logic_entt().get_or_emplace<AABB_centroid<Point_3> >(entity, aabb);

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

entt::entity object_3d_model(const std::string &name,
                             const std::string &mesh_path,
                             const Point_3 offset,
                             const Eigen::Quaternionf &rotate) {
    const entt::entity entity = Logic_entt().create();
    logic_create_proxy(entity);

    Logic_entt().emplace<Name_component>(entity, name);
    Logic_entt().emplace<Input_Component>(entity, model_3d_Event);


    add_shader(entity,
               "/Users/panxin/CLionProjects/hello_mac/render/shader/Phong.vert.spv",
               "/Users/panxin/CLionProjects/hello_mac/render/shader/pbr_bindless.frag.spv",
               "", "");
    auto aabb  = load_model(entity, mesh_path);
    auto &AABB = Logic_entt().get_or_emplace<AABB_centroid<Point_3> >(entity, aabb.value());

    // 更新物体的模型矩阵
    auto transform = Logic_entt().emplace<Transform>(entity, offset, rotate);

    const auto modelMatrix = get_model_matrix(transform);
    set_render_parameter(entity, "model_4x4", modelMatrix);

    world_root_add_child(entity);
    auto material = Logic_entt().get_or_emplace<PBR_component>(entity);
    set_render_parameter(entity, "object_material", material);

    logic_update_proxy<Name_component>(entity);
    logic_update_proxy(entity, get_VKR_mesh(entity));
    const auto primitives = create_primitives(entity);
    logic_update_proxy(entity, primitives);
    return entity;
}

entt::entity object_3d_model(const std::string &name, manifold::MeshGL &mesh, const Point_3 offset,
                             const Eigen::Quaternionf &rotate) {
    const entt::entity entity = Logic_entt().create();
    logic_create_proxy(entity);

    Logic_entt().emplace<Name_component>(entity, name);
    Logic_entt().emplace<Input_Component>(entity, model_3d_Event);


    add_shader(entity,
               "/Users/panxin/CLionProjects/hello_mac/render/shader/Phong.vert.spv",
               "/Users/panxin/CLionProjects/hello_mac/render/shader/Blinn_Phong_bindless.frag.spv",
               "", "");
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


    auto sp_vertices = std::make_shared<std::vector<Vertex> >();
    auto sp_indices  = std::make_shared<std::vector<uint16_t> >();
    sp_vertices->resize(mesh.vertProperties.size() / mesh.numProp);
    sp_indices->reserve(mesh.triVerts.size());
    memcpy(sp_vertices->data(), mesh.vertProperties.data(), sp_vertices->size() * sizeof(Vertex));
    for (int i = 0; i < mesh.triVerts.size(); i++) {
        sp_indices->push_back(mesh.triVerts.at(i));
    }
    add_geometry_data(entity, sp_vertices, sp_indices);
    auto [min, max] = find_min_max_point(sp_vertices);
    auto &AABB      = Logic_entt().get_or_emplace<AABB_centroid<Point_3> >(entity, AABB_centroid<Point_3>(min, max));

    // 更新物体的模型矩阵
    const auto transform = Logic_entt().emplace<Transform>(entity, offset, rotate);

    const auto modelMatrix = get_model_matrix(transform);
    set_render_parameter(entity, "model_4x4", modelMatrix);

    world_root_add_child(entity);

    logic_update_proxy<Name_component>(entity);
    logic_update_proxy(entity, get_VKR_mesh(entity));
    logic_update_proxy(entity, create_primitives(entity));
    return entity;
}


entt::entity add_sky_box(const std::string &name) {
    const entt::entity entity = Logic_entt().create();
    logic_create_proxy(entity);


    Logic_entt().emplace<Name_component>(entity, name);
    Logic_entt().emplace<Input_Component>(entity, model_3d_Event);


    add_shader(entity,
               "/Users/panxin/CLionProjects/hello_mac/render/shader/skybox.vert.spv",
               "/Users/panxin/CLionProjects/hello_mac/render/shader/skybox.frag.spv",
               "", "");
    add_box_data(entity);

    const auto mesh = get_VKR_mesh(entity);
    auto primitives = create_primitives(entity);
    for (auto &primitive: primitives) {
        primitive.set_front_face(VK_FRONT_FACE_COUNTER_CLOCKWISE);
        primitive.set_VkCullModeFlags(VK_CULL_MODE_FRONT_BIT);
    }
    logic_update_proxy(entity, mesh);
    logic_update_proxy(entity, primitives);


    // 更新物体的模型矩阵

    world_root_add_child(entity);

    logic_update_proxy<Name_component>(entity);

    return entity;
}


entt::entity object_3d_model(const std::string &name,
                             const AABB_min_max<Point_3> &bounding_box,
                             const Point_3 offset,
                             const Eigen::Quaternionf &rotate) {
    const entt::entity entity = Logic_entt().create();
    logic_create_proxy(entity);
    Logic_entt().emplace<Name_component>(entity, name);
    add_shader(entity,
               "/Users/panxin/CLionProjects/hello_mac/render/shader/Phong.vert.spv",
               "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_different_color.frag.spv",
               "", "");
    add_box_data(entity, bounding_box);

    // 更新物体的模型矩阵
    auto transform         = Logic_entt().emplace<Transform>(entity, offset, rotate);
    const auto modelMatrix = get_model_matrix(transform);
    set_render_parameter(entity, "model_4x4", modelMatrix);
    world_root_add_child(entity);
    logic_update_proxy<Name_component>(entity);
    logic_update_proxy(entity, get_VKR_mesh(entity));
    logic_update_proxy(entity, create_primitives(entity));
    logic_update_add_tag<opacity_tag>(entity);
    return entity;
}
