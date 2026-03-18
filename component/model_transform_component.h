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


/**
 * 构建 Vulkan 专用的透视投影矩阵
 *
 * 特点：
 * 1. 列优先存储 (Matches Vulkan/GLSL)
 * 2. Y 轴翻转 (Matches Vulkan ND-Coordinate System)
 * 3. Z 轴深度范围映射至 [0, 1] (Standard Vulkan depth)
 */
inline Eigen::Matrix4f vulkan_projection(float fovy_radians, float aspect, float zNear, float zFar) {
    // 强制使用列优先存储（虽然 Eigen 默认是 ColMajor，显式指定更安全）
    Eigen::Matrix4f projection = Eigen::Matrix4f::Zero();

    float tanHalfFovy = std::tan(fovy_radians / 2.0f);

    // 第一列：控制水平缩放
    projection(0, 0) = 1.0f / (aspect * tanHalfFovy);

    // 第二列：控制垂直缩放（注意这里的负号，用于翻转 Vulkan 的 Y 轴）
    projection(1, 1) = -1.0f / (tanHalfFovy);

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


class alignas(16) model_transform {
public:
    Eigen::Quaternionf rotate_ = {1, 0, 0, 0};
    Point_3 zoom_              = {1, 1, 1};
    Point_3 offset_            = {0, 0, 0};

    [[nodiscard]] Point_3 get_zoom() const {
        return zoom_;
    }

    explicit model_transform(const Point_3 offset) {
        offset_ = offset;
    }

    explicit model_transform(const Point_3 offset, const Eigen::Quaternionf &rotate) {
        offset_ = offset;
        rotate_ = rotate;
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

    static bool check_entity_intersect_point(entt::entity entity, const Point_2 &current_position) {
        if (auto *scene_node = Logic_entt().try_get<model_transform>(entity)) {
            // 下面这个3d部分是需要去写的，但是只能通过射线来进行检测了
            // if (intersect(scene_node->bounding_box_, current_position)) {
            // return true;
            // }
        }
        return false;
    }

    Eigen::Matrix4f get_view_projection() const {
        const auto view = view_matrix({offset_.x, offset_.y, offset_.z}, rotate_);
        return view;
    }
};


inline void update_object_offset() {
    const auto view = Logic_entt().view<UI_transform_dirty, Proxy_entity, model_transform>();
    // 包围盒发生了更新
    for (const auto it: view) {
        auto &transform  = view.get<model_transform>(it);
        auto modelMatrix = transform.update_model_matrix();
        set_render_parameter(it, "model_4x4", modelMatrix);
        Logic_entt().remove<UI_transform_dirty>(it);
    }
}


class camera_optical_component {
private:
    float fovy_radians = 45.0f;
    float aspect;
    float zNear = 0.1f;
    float zFar  = 1000.0f;

public:
    camera_optical_component() {
        const auto &backend  = VK_backend::get();
        auto [width, height] = backend.get_current_extent();
        aspect               = static_cast<float>(width) / static_cast<float>(height);
    }

    Eigen::Matrix4f get_projection() {
        const auto &handle    = VK_backend::get();
        auto [width, height]  = handle.get_current_extent();
        aspect                = static_cast<float>(width) / static_cast<float>(height);
        const auto projection = vulkan_projection(to_radians(fovy_radians),
                                                  aspect,
                                                  zNear,
                                                  zFar);
        return projection;
    }
};


inline void update_camera_transform() {
    const auto view = Logic_entt().view<Camera_transform_dirty, Name_component, model_transform>();
    for (const auto it: view) {
        auto &camera_pos = view.get<model_transform>(it);
        auto &name       = view.get<Name_component>(it);
        if (name.name.find("world_scene_root") != std::string::npos) {
            const auto view_matrix = camera_pos.get_view_projection();
            set_render_parameter(it, "global_view_4x4", view_matrix);
            Point_3 world_camera_pos = camera_pos.get_offset();
            const Point_3 world_light_pos{0, 10, 6};

            set_render_parameter(it, "global_world_view_Pos", world_camera_pos);
            set_render_parameter(it, "global_world_light_Pos", world_light_pos);
        }
        Logic_entt().remove<Camera_transform_dirty>(it);
    }
}

inline void update_camera_optical() {
    const auto view = Logic_entt().view<Camera_optical_specifications_dirty, camera_optical_component,
                                        Name_component>();
    for (const auto it: view) {
        auto &name    = view.get<Name_component>(it);
        auto &optical = view.get<camera_optical_component>(it);
        if (name.name.find("world_scene_root") != std::string::npos) {
            const auto view_matrix = optical.get_projection();
            set_render_parameter(it, "global_projection_4x4", view_matrix);
        }
        Logic_entt().remove<Camera_optical_specifications_dirty>(it);
    }
}

class world_scene_root {
public:
    // 获取全局唯一的注册表引用
    static entt::entity &get() {
        static entt::entity instance = Logic_entt().create();;
        static std::once_flag flag;

        std::call_once(flag, []() {
                           Logic_entt().emplace<Scene_Component>(instance);
                           Logic_entt().emplace<Name_component>(instance, "world_scene_root");
                           Logic_entt().emplace<VKR_shader_paths>(instance,
                                                                  "/Users/panxin/CLionProjects/hello_mac/render/shader/multiple_render_targets.vert.spv",
                                                                  "/Users/panxin/CLionProjects/hello_mac/render/shader/multiple_render_targets.frag.spv",
                                                                  "", "");
                           auto camera           = Logic_entt().get_or_emplace<camera_optical_component>(instance);
                           const auto projection = camera.get_projection();
                           const Point_3 world_light_pos{0, 10, 6};

                           const auto camera_pos = Logic_entt().get_or_emplace<model_transform>(instance, Point_3{
                                        0, 0, 6
                                    });
                           const auto view_matrix   = camera_pos.get_view_projection();
                           Point_3 world_camera_pos = camera_pos.get_offset();


                           set_render_parameter(instance, "global_projection_4x4", projection);
                           set_render_parameter(instance, "global_view_4x4", view_matrix);
                           set_render_parameter(instance, "global_world_view_Pos", world_camera_pos);
                           set_render_parameter(instance, "global_world_light_Pos", world_light_pos);
                       }
                      );
        return instance;
    }

private:
    world_scene_root() = default; // 禁用构造
};


static entt::entity &get_world_root() {
    return world_scene_root::get();
}


#endif //HELLO_MAC_RENDER_COMPONENT_H
