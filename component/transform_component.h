//
// Created by 潘鑫 on 2025/12/23.
//

#ifndef HELLO_MAC_RENDER_COMPONENT_H
#define HELLO_MAC_RENDER_COMPONENT_H

#include "base_geometry/base.h"
#include <Eigen/Eigen>


struct Transform_matrix_dirty {
};

class alignas(16) Transform_matrix {
public:
    explicit Transform_matrix(const Eigen::Matrix4f &modelMatrix = Eigen::Matrix4f::Identity()) {
        model_Matrix_ = modelMatrix;
    }

    Eigen::Matrix4f &get() {
        return model_Matrix_;
    }

private:
    Eigen::Matrix4f model_Matrix_;
};

class alignas(16) Transform {
    Eigen::Quaternionf rotate_ = {1, 0, 0, 0};
    Point_3 zoom_              = {1, 1, 1};
    Point_3 position_          = {0, 0, 0};

public:
    explicit Transform(const Eigen::Matrix4f matrix) {
    }

    explicit Transform(const Point_3 position,
                       const Eigen::Quaternionf &rotate = {1, 0, 0, 0},
                       const Point_3 zoom               = {1, 1, 1}) {
        position_ = position;
        rotate_   = rotate;
        zoom_     = zoom;
    }


    [[nodiscard]] Eigen::Quaternionf get_rotate() const {
        return rotate_;
    }

    [[nodiscard]] Point_3 get_zoom() const {
        return zoom_;
    }

    [[nodiscard]] Point_3 get_position() const {
        return position_;
    }

    Point_3 add_offset(const Point_3 offset) {
        return position_ = position_ + offset;
    }

    Eigen::Quaternionf mult_rotate(const Eigen::Quaternionf &quaternion) {
        return rotate_ = rotate_ * quaternion; // multiply
    }

    Eigen::Quaternionf set_rotate(const Eigen::Quaternionf &quaternion) {
        return rotate_ = quaternion;
    }

    [[nodiscard]] Eigen::Matrix4f get_transform_matrix() const {
        // 定义一个仿射变换（4x4 矩阵）
        Eigen::Affine3f model_4x4 = Eigen::Affine3f::Identity();
        // 1. 平移 (Translation)
        model_4x4.translate(Eigen::Vector3f(position_.x, position_.y, position_.z));
        // 2. 旋转 (Rotation) - 使用四元数
        model_4x4.rotate(rotate_);
        // 3. 缩放 (Scaling)
        model_4x4.scale(Eigen::Vector3f(zoom_.x, zoom_.y, zoom_.z));
        // 获取最终传给 Vulkan 的 4x4 矩阵
        Eigen::Matrix4f modelMatrix = model_4x4.matrix();
        return modelMatrix;
    }
};


[[nodiscard]] Eigen::Matrix4f get_model_matrix(const Transform transform);

[[nodiscard]] Eigen::Matrix4f get_model_matrix(const AABB_min_max<Point_3> &bound_box, const Transform transform);

void update_camera_transform();


#endif //HELLO_MAC_RENDER_COMPONENT_H
