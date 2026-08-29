//
// Created by 潘鑫 on 2026/8/22.
//

#include "object_ply.h"

#include "name_component.h"
#include "mesh_component.h"
#include "shader_component.h"
#include <Eigen/Eigen>

#include "base_3d_render_object.h"
#include "transform_component.h"

#include "B_spline_cureve.h"
#include "../render/render_common/PBR_component.h"
#include "world_scene_root.h"
#include "PLYLoader.h"

VKR_shader_paths get_gltf_shader_path();


object_3DGS::object_3DGS(const std::string &name) : logic_render_object(name) {
    logic_create_proxy(entity);
    logic_update_proxy<Name_component>(entity);
    world_root_add_child(entity);
}

void object_3DGS::add_model(const std::string &file_path) {
    int _degree = 0;
    std::unique_ptr<GaussianBase> gaussianData = PLYLoader::LoadPLY(file_path, _degree);
    auto min_max = gaussianData->get_min_max();
    auto &parameters = Logic_entt().emplace_or_replace<object_3DGS_parameters>(entity);
    parameters.positions_ptr = copy_data_to_SSBO_buffer(gaussianData->_xyz);
    parameters.scales_ptr = copy_data_to_SSBO_buffer(gaussianData->_scales);
    parameters.rotations_ptr = copy_data_to_SSBO_buffer(gaussianData->_rotations);
    parameters.opacities_ptr = copy_data_to_SSBO_buffer(gaussianData->_opacities);
    parameters.sh_coefficients_ptr = copy_data_to_SSBO_buffer(gaussianData->_shCoefficients);
    parameters.near = 1;
    parameters.far = 1000;
    parameters.gaussianCount = gaussianData->_numGaussians;
    parameters.shDegree = gaussianData->_shDegree;
    parameters.radius_ptr = copy_data_to_SSBO_buffer(nullptr, parameters.gaussianCount * sizeof(int));
    parameters.depth_ptr = copy_data_to_SSBO_buffer(nullptr, parameters.gaussianCount * sizeof(float));
    parameters.rgb_ptr = copy_data_to_SSBO_buffer(nullptr, parameters.gaussianCount * sizeof(float) * 4);
    parameters.conicOpacity_ptr = copy_data_to_SSBO_buffer(nullptr, parameters.gaussianCount * sizeof(float) * 4);
    parameters.pointsXY_ptr = copy_data_to_SSBO_buffer(nullptr, parameters.gaussianCount * sizeof(float) * 2);
    parameters.tilesTouched_ptr = copy_data_to_SSBO_buffer(nullptr, parameters.gaussianCount * sizeof(uint32_t));
    parameters.tilesTouched_Prefix_Sum_ptr =
            copy_data_to_SSBO_buffer(nullptr, parameters.gaussianCount * sizeof(uint32_t));
    parameters.bbox_ptr       = copy_data_to_SSBO_buffer(nullptr, parameters.gaussianCount * sizeof(float) * 4);
    parameters.keysUnsorted   = copy_data_to_SSBO_buffer(nullptr, 50000000 * sizeof(uint64_t));
    parameters.valuesUnsorted = copy_data_to_SSBO_buffer(nullptr, 50000000 * sizeof(uint32_t));
    parameters.culling        = 1;
    parameters.update_gpu_addresses();
    logic_update_proxy(entity, parameters);

    logic_update_add_tag<ply_3DGS_tag>(entity);
}


entt::entity object_ply_model(const std::string &name, const std::string &file_path, const Eigen::Vector3f offset,
                              const Eigen::Quaternionf &rotate) {
    const entt::entity entity = Logic_entt().create();
    int _degree               = 0;
    auto _gaussianData        = PLYLoader::LoadPLY(file_path, _degree);

    auto min_max = _gaussianData->get_min_max();
    // 全部的点云文件拿到了,之后呢? 我需要先知道它的大小

    // object_3d box("3dGS_lpy");
    // box.add_shader_path(get_gltf_shader_path());
    // box.set_random_triangle_color();
    // box.add_mesh({
    //                  {min_max.first.x(), min_max.first.y(), min_max.first.z()},
    //                  {min_max.second.x(), min_max.second.y(), min_max.second.z()}
    //              });
    // box.set_transform();


    // 更新物体的模型矩阵
    // auto transform = Logic_entt().emplace<Transform>(entity, offset, rotate);

    // const auto modelMatrix = get_model_matrix(transform);
    // set_render_parameter(entity, "model_4x4", modelMatrix);

    // world_root_add_child(entity);
    // auto material = Logic_entt().get_or_emplace<PBR_component>(entity);
    // set_render_parameter(entity, "object_material", material);

    // logic_update_proxy<Name_component>(entity);
    // logic_update_proxy(entity, get_VKR_mesh(entity));
    // logic_update_proxy(entity, create_primitives(entity));
    return entity;
}
