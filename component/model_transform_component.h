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
    Eigen::Quaternionf rotate_ = {1, 0, 0, 0};
    Point_3 zoom_              = {1, 1, 1};
    Point_3 offset_            = {0, 0, 0};

public:
    explicit model_transform(const Point_3 offset) {
        offset_ = offset;
    }

    explicit model_transform(const Point_3 offset, const Eigen::Quaternionf &rotate) {
        offset_ = offset;
        rotate_ = rotate;
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


inline Ray<Point_3> &get_screen_ray(const Point_2 mouse_positon) {
    static Point_2 last_mouse_position = {0, 0};
    static Ray<Point_3> last_ray       = {{0, 0, 0}, {0, 0, 1}};
    if (mouse_positon == last_mouse_position) {
        return last_ray;
    }
    last_mouse_position = mouse_positon;
    // 上面是一个简短记忆上一次的代码

    auto world_entity     = get_world_root();
    auto camera           = Logic_entt().try_get<camera_optical_component>(world_entity);
    const auto camera_pos = Logic_entt().try_get<model_transform>(world_entity);

    const auto &backend    = VK_backend::get();
    auto [width, height]   = backend.get_current_extent();
    const auto projection  = camera->get_projection();
    const auto view_matrix = camera_pos->get_view_projection();

    // 1. 转换到 NDC 坐标 (假设鼠标坐标为 mouseX, mouseY)
    // 这里有一个坑，gltf 给出的坐标和拿到的 显示区域的宽和高差两倍
    float x = (4.0f * mouse_positon.x) / width - 1.0f;
    float y = (4.0f * mouse_positon.y) / height - 1.0f; // 注意：Vulkan/GLFW 的 Y 轴通常需要反转

    // 2. 构造近裁剪面和远裁剪面的点 (在裁剪空间)
    // Vulkan 的近平面通常是 z=0.0，远平面是 z=1.0
    Eigen::Vector4f ray_start_clip(x, y, 0.0f, 1.0f);
    Eigen::Vector4f ray_end_clip(x, y, 1.0f, 1.0f);

    // 3. 计算逆矩阵
    Eigen::Matrix4f invVP = (projection * view_matrix).inverse();

    // 4. 转换回世界空间
    Eigen::Vector4f world_start = invVP * ray_start_clip; // 这里给出来的是近平面上的起始点
    Eigen::Vector4f world_end   = invVP * ray_end_clip;

    // 5. 透视除法 (W 分量归一化)
    world_start /= world_start.w();
    world_end   /= world_end.w();
    auto offset = camera_pos->get_offset(); // 这里给出的相机的位置
    // 6. 确定射线
    Eigen::Vector3f ray_origin    = world_start.head<3>();
    Eigen::Vector3f ray_direction = (world_end.head<3>() - ray_origin).normalized();
    last_ray                      = {
        {offset},
        {ray_direction.x(), ray_direction.y(), ray_direction.z()}
    };
    return last_ray;
}


#endif //HELLO_MAC_RENDER_COMPONENT_H
