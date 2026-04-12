//
// Created by 潘鑫 on 2025/12/23.
//

#ifndef HELLO_MAC_RENDER_COMPONENT_H
#define HELLO_MAC_RENDER_COMPONENT_H


#include <scene_component.h>

#include "name_component.h"
#include "render_proxy.h"
#include "shader_component.h"
#include "VKR_proxy_component.h"
#include <Eigen/Eigen>


/**
 * 使用位置和四元数构建 View 矩阵
 * 适配 Vulkan (列优先)
 */
inline Eigen::Matrix4f view_matrix(const Eigen::Vector3f &pos, const Eigen::Quaternionf &q) {
    // 1. 将四元数转换为旋转矩阵（Eigen 会自动处理归一化并使用 NEON 加速）
    // 注意：View 矩阵需要的是相机的逆旋转
    Eigen::Matrix3f R = q.toRotationMatrix().transpose();

    // 2. 计算平移部分：-(R * pos)
    Eigen::Vector3f t = -(R * pos);

    // 3. 组合成 4x4 矩阵
    Eigen::Matrix4f view   = Eigen::Matrix4f::Identity();
    view.block<3, 3>(0, 0) = R;
    view.block<3, 1>(0, 3) = t;

    return view;
}


class alignas(16) model_transform {
    Eigen::Quaternionf rotate_ = {1, 0, 0, 0};
    Point_3 zoom_              = {1, 1, 1};
    Point_3 offset_            = {0, 0, 0};

public:
    explicit model_transform(const Eigen::Matrix4f matrix) {
    }

    explicit model_transform(const Point_3 offset) {
        offset_ = offset;
    }

    explicit model_transform(const Point_3 offset, const Eigen::Quaternionf &rotate) {
        offset_ = offset;
        rotate_ = rotate;
    }

    explicit model_transform(const Point_3 offset, const Eigen::Quaternionf &rotate, const Point_3 zoom) {
        offset_ = offset;
        rotate_ = rotate;
        zoom_   = zoom;
    }


    [[nodiscard]] Eigen::Quaternionf get_rotate() const {
        return rotate_;
    }

    [[nodiscard]] Point_3 get_zoom() const {
        return zoom_;
    }

    [[nodiscard]] Point_3 get_offset() const {
        return offset_;
    }

    Point_3 add_offset(const Point_3 offset_add) {
        return offset_ = offset_ + offset_add;
    }

    void rotate(const Eigen::Quaternionf &quaternion) {
        rotate_ = rotate_ * quaternion;
    }

    void set_rotate(const Eigen::Quaternionf &quaternion) {
        rotate_ = quaternion;
    }

    Eigen::Matrix4f update_model_matrix() const {
        // 定义一个仿射变换（4x4 矩阵）
        Eigen::Affine3f model_4x4 = Eigen::Affine3f::Identity();
        // 1. 平移 (Translation)
        model_4x4.translate(Eigen::Vector3f(offset_.x, offset_.y, offset_.z));
        // 2. 旋转 (Rotation) - 使用四元数
        model_4x4.rotate(rotate_);
        // 3. 缩放 (Scaling)
        model_4x4.scale(Eigen::Vector3f(zoom_.x, zoom_.y, zoom_.z));
        // 获取最终传给 Vulkan 的 4x4 矩阵
        Eigen::Matrix4f modelMatrix = model_4x4.matrix();
        return modelMatrix;
    }

    Eigen::Matrix4f get_view_projection() const {
        const auto view = view_matrix({offset_.x, offset_.y, offset_.z}, rotate_);
        return view;
    }
};


void update_camera_transform();


void init_world_scene_root(entt::entity instance);

class world_scene_root {
public:
    // 获取全局唯一的注册表引用
    static entt::entity &get() {
        static entt::entity instance = Logic_entt().create();;
        static std::once_flag flag;
        std::call_once(flag, []() {
                           init_world_scene_root(instance);
                       }
                      );
        return instance;
    }

private:
    world_scene_root() = default; // 禁用构造
};


inline entt::entity &get_world_root() {
    return world_scene_root::get();
}


Ray<Point_3> &get_screen_ray(const Point_2 mouse_positon);


wmOperatorStatus model_3d_Event(const entt::entity entity, const base_event_with_stamp &event);


uint32_t free_bindless_uniform_sampler2D(const std::string &name);

uint32_t add_bindless_uniform_sampler2D(const std::string &name,
                                        std::optional<Texture_parameter> &update);
#endif //HELLO_MAC_RENDER_COMPONENT_H
