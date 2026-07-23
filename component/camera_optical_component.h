//
// Created by 潘鑫 on 2026/3/19.
//

#ifndef HELLO_MAC_CAMERA_OPTICAL_COMPONENT_H
#define HELLO_MAC_CAMERA_OPTICAL_COMPONENT_H
#include "vulkan_backend.h"
#include <Eigen/Eigen>

#include "name_component.h"
#include "shader_component.h"
#include "sync_proxy_to_render_thread.h"


/**
 * 构建 Vulkan 专用的透视投影矩阵
 *
 * 特点：
 * 1. 列优先存储 (Matches Vulkan/GLSL)
 * 2. Y 轴翻转 (Matches Vulkan ND-Coordinate System)
 * 3. Z 轴深度范围映射至 [0, 1] (Standard Vulkan depth)
 */
inline Eigen::Matrix4f vulkan_projection(float fovy_radians, float aspect, float zNear, float zFar,
                                         bool flip_y_axis = true) {
    // 强制使用列优先存储（虽然 Eigen 默认是 ColMajor，显式指定更安全）
    Eigen::Matrix4f projection = Eigen::Matrix4f::Zero();

    float tanHalfFovy = std::tan(fovy_radians / 2.0f);

    // 第一列：控制水平缩放
    projection(0, 0) = 1.0f / (aspect * tanHalfFovy);

    // 第二列：控制垂直缩放（注意这里的负号，用于翻转 Vulkan 的 Y 轴）
    if (flip_y_axis == true)
        projection(1, 1) = -1.0f / (tanHalfFovy);
    else
        projection(1, 1) = 1.0f / (tanHalfFovy);


    // 第三列：控制 Z 轴深度映射及 W 分量
    // 映射 [zNear, zFar] 到 [0, 1]
    projection(2, 2) = zFar / (zNear - zFar);
    projection(3, 2) = -1.0f; // 用于透视除法

    // 第四列：控制 Z 轴平移
    projection(2, 3) = -(zFar * zNear) / (zFar - zNear);

    return projection;
}

template<typename T>
T to_radians(T degrees) {
    return degrees * (EIGEN_PI / T(180));
}


class camera_optical_component {
private:
    float fovy_radians_ = 45.0f;
    float aspect_;
    float zNear_ = 0.1f;
    float zFar_  = 1000.0f;

public:
    camera_optical_component() {
        const auto &backend  = VK_backend::instance();
        auto [width, height] = backend.get_current_extent();
        aspect_              = static_cast<float>(width) / static_cast<float>(height);
    }

    Eigen::Matrix4f get_projection_matrix() {
        const auto &handle    = VK_backend::instance();
        auto [width, height]  = handle.get_current_extent();
        aspect_               = static_cast<float>(width) / static_cast<float>(height);
        const auto projection = vulkan_projection(to_radians(fovy_radians_),
                                                  aspect_,
                                                  zNear_,
                                                  zFar_);
        return projection;
    }
};


inline void update_camera_optical() {
    const auto view = Logic_entt().view<Camera_optical_specifications_dirty, camera_optical_component,
                                        Name_component>();
    for (const auto it: view) {
        auto &name    = view.get<Name_component>(it);
        auto &optical = view.get<camera_optical_component>(it);
        if (name.name_.find("world_scene_root") != std::string::npos) {
            const auto projection_matrix = optical.get_projection_matrix();
            // set_render_parameter(it, "global_projection_4x4", projection_matrix);
            Eigen::Matrix4f inv_projection_matrix = projection_matrix.inverse();
            // set_render_parameter(it, "global_inv_projection_4x4", inv_projection_matrix);
        }
        Logic_entt().remove<Camera_optical_specifications_dirty>(it);
    }
}

#endif //HELLO_MAC_CAMERA_OPTICAL_COMPONENT_H
