//
// Created by 潘鑫 on 2026/3/6.
//

#ifndef HELLO_MAC_3D_MODEL_DISPLAY_H
#define HELLO_MAC_3D_MODEL_DISPLAY_H

#include "global_singleton.h"
#include "base_geometry/base.h"
#include "model_transform_component.h"


entt::entity object_3d_model(const std::string &name, const std::string &mesh_path, const Point_3 offset,
                             const Eigen::Quaternionf &rotate = Eigen::Quaternionf::Identity());


inline Ray<Point_3> get_screen_ray(Point_2 x_y) {
    auto world_entity     = get_world_root();
    auto camera           = Logic_entt().try_get<camera_optical_component>(world_entity);
    const auto camera_pos = Logic_entt().try_get<model_transform>(world_entity);

    const auto &backend    = VK_backend::get();
    auto [width, height]   = backend.get_current_extent();
    const auto projection  = camera->get_projection();
    const auto view_matrix = camera_pos->get_view_projection();

    // 1. 转换到 NDC 坐标 (假设鼠标坐标为 mouseX, mouseY)
    // 这里有一个坑，gltf 给出的坐标和拿到的 显示区域的宽和高差两倍
    float x = (4.0f * x_y.x) / width - 1.0f;
    float y = (4.0f * x_y.y) / height - 1.0f; // 注意：Vulkan/GLFW 的 Y 轴通常需要反转

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
    Ray<Point_3> result           = {
        {offset},
        {ray_direction.x(), ray_direction.y(), ray_direction.z()}
    };
    return result;
}


#endif //HELLO_MAC_3D_MODEL_DISPLAY_H
