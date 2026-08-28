//
// Created by 潘鑫 on 2026/8/22.
//

#ifndef HELLO_MAC_OBJECT_PLY_H
#define HELLO_MAC_OBJECT_PLY_H

#include "global_singleton.h"
#include <Eigen/Eigen>

#include "base_render_object.h"


class object_3DGS : public logic_render_object {
public:
    object_3DGS(const std::string &name);

    void add_model(const std::string &file_path);
};

entt::entity object_ply_model(const std::string &name,
                              const std::string &file_path,
                              const Eigen::Vector3f offset     = Eigen::Vector3f::Zero(),
                              const Eigen::Quaternionf &rotate = Eigen::Quaternionf());


class object_3DGS_parameters {
public:
    uint64_t positions_address;
    uint64_t scales_address;
    uint64_t rotations_address;
    uint64_t opacities_address;
    uint64_t sh_coefficients_address;

    uint64_t radius_address;
    uint64_t depth_address;
    uint64_t rgb_address;
    uint64_t conicOpacity_address;
    uint64_t pointsXY_address;
    uint64_t tilesTouched_address;
    uint64_t bbox_address;

    float near;
    float far;
    uint gaussianCount;
    uint culling;
    int shDegree;

    VKR_buffer_ptr positions_ptr;
    VKR_buffer_ptr scales_ptr;
    VKR_buffer_ptr rotations_ptr;
    VKR_buffer_ptr opacities_ptr;
    VKR_buffer_ptr sh_coefficients_ptr;
    VKR_buffer_ptr radius_ptr;
    VKR_buffer_ptr depth_ptr;
    VKR_buffer_ptr rgb_ptr;
    VKR_buffer_ptr conicOpacity_ptr;
    VKR_buffer_ptr pointsXY_ptr;
    VKR_buffer_ptr tilesTouched_ptr;
    VKR_buffer_ptr bbox_ptr;
    VKR_buffer_ptr tilesTouched_Prefix_Sum_ptr;


    void update_gpu_addresses() {
        positions_address       = positions_ptr->get_gpu_device_address();
        scales_address          = scales_ptr->get_gpu_device_address();
        rotations_address       = rotations_ptr->get_gpu_device_address();
        opacities_address       = opacities_ptr->get_gpu_device_address();
        sh_coefficients_address = sh_coefficients_ptr->get_gpu_device_address();
        radius_address          = radius_ptr->get_gpu_device_address();
        depth_address           = depth_ptr->get_gpu_device_address();
        rgb_address             = rgb_ptr->get_gpu_device_address();
        conicOpacity_address    = conicOpacity_ptr->get_gpu_device_address();
        pointsXY_address        = pointsXY_ptr->get_gpu_device_address();
        tilesTouched_address    = tilesTouched_ptr->get_gpu_device_address();
        bbox_address            = bbox_ptr->get_gpu_device_address();
    }
};

#endif //HELLO_MAC_OBJECT_PLY_H
