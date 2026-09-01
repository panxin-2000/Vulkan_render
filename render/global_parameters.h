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
    std::array<FrustumPlanes, 4> light_frustum_planes;
    Eigen::Vector4f world_camera_pos;
    Light light;
    Eigen::Vector4f screen_size;
    float split_depth[4];
    float film_grain_intensity       = 0.2f;
    float camera_vignette_smoothness = 0.2f;
    float camera_vignette_intensity  = 0.4f;
    uint32_t render_timeline;
    std::array<Eigen::Array4f, 9> shCoefficients;

    float &get_camera_vignette_smoothness() {
        return camera_vignette_smoothness;
    }

    float &get_camera_vignette_intensity() {
        return camera_vignette_intensity;
    }

    float &get_film_grain_intensity() {
        return film_grain_intensity;
    }

    bool set_render_timeline(const uint32_t &timeline);

    bool set_projection_matrix(const Eigen::Matrix4f &matrix);

    bool set_inv_projection_matrix(const Eigen::Matrix4f &matrix);

    bool set_view_matrix(const Eigen::Matrix4f &matrix);

    bool set_inv_view_matrix(const Eigen::Matrix4f &matrix);

    bool set_invVP(const Eigen::Matrix4f &matrix);

    bool set_world_camera_pos(const Eigen::Vector3f &v3);

    bool set_sun_light(const Eigen::Vector3f &v3);

    bool set_screen_size(const Eigen::Vector2f &screen_size_t);

    bool update_directional_light();;
};


#endif //HELLO_MAC_GLOBAL_PARAMETERS_H
