//
// Created by 潘鑫 on 2026/8/24.
//

#ifndef HELLO_MAC_GLOBAL_PARAMETERS_H
#define HELLO_MAC_GLOBAL_PARAMETERS_H
#include <Eigen/Eigen>
#include "frustum.h"
#include "PBR_component.h"


class Global_parameters {
public:
    Eigen::Matrix4f view_matrix;
    Eigen::Matrix4f projection_matrix;
    Eigen::Matrix4f inv_view_matrix;
    Eigen::Matrix4f inv_projection_matrix;
    Eigen::Matrix4f invVP;
    Eigen::Matrix4f light_viewProjMatrix[4];
    FrustumPlanes frustum_planes;
    Eigen::Vector4f world_camera_pos;
    Light light;
    Eigen::Vector4f screen_size;
    std::array<Eigen::Array4f, 9> shCoefficients;
    Eigen::Vector4f cascadeSplits;

    bool set_projection_matrix(const Eigen::Matrix4f &matrix);

    bool set_inv_projection_matrix(const Eigen::Matrix4f &matrix);

    bool set_view_matrix(const Eigen::Matrix4f &matrix);

    bool set_inv_view_matrix(const Eigen::Matrix4f &matrix);

    bool set_invVP(const Eigen::Matrix4f &matrix);

    bool set_world_camera_pos(const Eigen::Vector3f &v3);

    bool set_sun_light(const Eigen::Vector3f &v3);

    bool set_screen_size(const Eigen::Vector2f &screen_size_t);

    bool update_directional_light(const Eigen::Vector3f &v3);;
};


#endif //HELLO_MAC_GLOBAL_PARAMETERS_H
