//
// Created by 潘鑫 on 2025/12/23.
//

#ifndef HELLO_MAC_RENDER_COMPONENT_H
#define HELLO_MAC_RENDER_COMPONENT_H

#include "base_geometry/base.h"
#include <Eigen/Eigen>

#include "AABB_box.h"
#include "global_singleton.h"


struct Transform_matrix_dirty {
};


class alignas(16) Transform_Matrix : public Eigen::Matrix4f {
};

class alignas(16) Transform {
    Eigen::Quaternionf rotate_ = {1, 0, 0, 0};
    Eigen::Vector3f zoom_      = {1, 1, 1};
    Eigen::Vector3f offset_    = {0, 0, 0};

public:
    explicit Transform(const Eigen::Matrix4f matrix) {
    }

    explicit Transform(const Eigen::Vector3f offset,
                       const Eigen::Quaternionf &rotate = {1, 0, 0, 0},
                       const Eigen::Vector3f zoom       = {1, 1, 1}) {
        offset_ = offset;
        rotate_ = rotate;
        zoom_   = zoom;
    }


    [[nodiscard]] Eigen::Quaternionf get_rotate() const {
        return rotate_;
    }

    [[nodiscard]] auto get_zoom() const {
        return zoom_;
    }

    [[nodiscard]] auto get_offset() const {
        return offset_;
    }

    auto add_offset(const Eigen::Vector3f offset) {
        return offset_ = offset_ + offset;
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
        model_4x4.translate(offset_);
        // 2. 旋转 (Rotation) - 使用四元数
        model_4x4.rotate(rotate_);
        // 3. 缩放 (Scaling)
        model_4x4.scale(zoom_);
        // 获取最终传给 Vulkan 的 4x4 矩阵
        Eigen::Matrix4f modelMatrix = model_4x4.matrix();
        return modelMatrix;
    }
};

class Local_Space_AABB : public Render_AABB {
};


class World_Space_AABB : public Render_AABB_min {
};

Render_AABB transform_AABB(const Render_AABB &bound_box, const Eigen::Matrix4f &matrix);

[[nodiscard]] Eigen::Matrix4f get_model_matrix(const Transform transform);

[[nodiscard]] Eigen::Matrix4f get_model_matrix(const AABB_min_max<Point_3> &bound_box, const Transform transform);

void update_camera_transform();

void update_transform_matrix(const entt::entity entity);

void set_transform_dirty(const entt::entity entity);

void update_primitives_model_matrix(const entt::entity model_entity);

void update_primitives_model_box(const entt::entity model_entity);

#endif //HELLO_MAC_RENDER_COMPONENT_H
