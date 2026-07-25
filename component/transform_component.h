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

[[nodiscard]] Point_3 get_view_direction(const Transform &transform);

[[nodiscard]] Point_3 get_view_right_direction(const Transform &transform);

[[nodiscard]] Eigen::Matrix4f get_model_matrix(const Transform transform);

[[nodiscard]] Eigen::Matrix4f get_model_matrix(const AABB_min_max<Point_3> &bound_box, const Transform transform);

[[nodiscard]] Eigen::Matrix4f get_view_matrix(const Transform transform);

void update_camera_transform();


void init_world_scene_root(entt::entity entity);

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


wmOperatorStatus model_3d_Event(const entt::entity entity, const SDL_Event &event);


uint32_t free_bindless_uniform_sampler2D(const std::string &name);

#endif //HELLO_MAC_RENDER_COMPONENT_H
