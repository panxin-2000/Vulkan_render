//
// Created by 潘鑫 on 2026/7/29.
//

#ifndef HELLO_MAC_FRUSTUM_H
#define HELLO_MAC_FRUSTUM_H


#include <Eigen/Eigen>
#include <vector>
#include "AABB_box.h"

struct FrustumPlanes {
    std::array<Eigen::Vector4f, 6> planes = {};
};


inline FrustumPlanes get_Frustum_Planes(Eigen::Matrix4f vp) {
    // 1. 计算 View-Projection 复合矩阵
    FrustumPlanes frustum_planes;
    // 2. 提取 4 个行向量 (Row Vectors)
    const Eigen::Vector4f r0 = vp.row(0);
    const Eigen::Vector4f r1 = vp.row(1);
    const Eigen::Vector4f r2 = vp.row(2);
    const Eigen::Vector4f r3 = vp.row(3);

    // 声明 6 个平面的临时存储
    std::vector<Eigen::Vector4f> raw_planes(6);

    // 3. 【Vulkan 专属公式组合】
    raw_planes[0] = r3 + r0; // 左平面 (Left)
    raw_planes[1] = r3 - r0; // 右平面 (Right)
    raw_planes[2] = r3 + r1; // 下平面 (Bottom)
    raw_planes[3] = r3 - r1; // 上平面 (Top)
    raw_planes[4] = r2;      // 近平面 (Near): Vulkan 裁剪空间中 z = 0
    raw_planes[5] = r3 - r2; // 远平面 (Far): Vulkan 裁剪空间中 z = w


    // 4. 对 6 个平面进行严谨的数学归一化
    for (int i = 0; i < 6; ++i) {
        // 提取平面法向量 (A, B, C) 并计算模长
        const float length = raw_planes[i].head<3>().norm();

        // 防除以 0 保护（针对退化矩阵）
        float inv_length = (length > 0.0f) ? (1.0f / length) : 1.0f;

        // 归一化整个 vec4 (A, B, C, D)
        frustum_planes.planes[i] = raw_planes[i] * inv_length;
    }
    return frustum_planes;
}


// inline bool frustum_cull(const FrustumPlanes &frustum_planes,
//                          const AABB_min_max<Point_3> &bounds,
//                          const Eigen::Vector4f &camera_pos) {
//     // 根据 视锥裁切平面 法向量 ， 找到 包围盒 中 距离 平面最近的点， 判断 是否在视锥范围内
//     Eigen::Vector3f hi{bounds.max_point_.x, bounds.max_point_.y, bounds.max_point_.z};
//     Eigen::Vector3f lo{bounds.min_point_.x, bounds.min_point_.y, bounds.min_point_.z};
//
//     bool is_camera_inside = (camera_pos.x() >= lo.x() && camera_pos.x() <= hi.x()) &&
//                             (camera_pos.y() >= lo.y() && camera_pos.y() <= hi.y()) &&
//                             (camera_pos.z() >= lo.z() && camera_pos.z() <= hi.z());
//
//     if (is_camera_inside) {
//         return true; // 🌟 相机在物体内部，绝对可见，直接熔断返回！
//     }
//
//     float is_visible = 1.0f;
//     for (int i = 0; i < 6; i++) {
//         // 获取当前平面的系数。p.xyz 是法向量，p.w 是从原点到平面的距离
//         Eigen::Vector4f p         = frustum_planes.planes[i];
//         auto p_xyz                = p.head<3>();
//         auto high_mask            = (p_xyz.array() > 0.0f);
//         Eigen::Vector3f max_coord = high_mask.select(hi, lo);
//         float distance            = max_coord.dot(p.head<3>()) + p.w();
//         is_visible                *= (distance >= -0.0001f) ? 1.0f : 0.0f;
//     }
//     return is_visible > 0.5f;
// }


inline bool frustum_cull_2(const FrustumPlanes &frustum_planes,
                           const Render_AABB &bounds,
                           const Eigen::Vector4f &camera_pos) {
    // 包围盒的
    // bounds.centroid_points      最后一个分量为1
    // bounds.direction_intervals  最后一个分量为0
    bool all_planes_inside = true;

    // 强制编译器展开循环，消除循环开销
#pragma unroll
    for (int i = 0; i < 6; ++i) {
        const Eigen::Vector4f &p = frustum_planes.planes[i];
        // 2. 【核心优化】计算 AABB 沿平面法线的最大正向投影半径
        // .cwiseAbs() 会对法向量的每个分量取绝对值
        // .dot() 执行极致的 SIMD 乘加运算，彻底代替了原本的 mix 掩码操作
        float projectedRadius = bounds.direction_intervals.dot(p.cwiseAbs());
        // 3. 计算中心点到平面的带符号物理距离
        const float distanceToCenter = bounds.centroid_points.dot(p);
        if (distanceToCenter < -projectedRadius - 0.0001f) {
            return false; // 整个盒体完全在平面外侧，安全剔除  // 有时很快,有时很慢, 是因为这里有快捷返回
        }
    }
    // 如果 6 个平面都认为盒子完全在内侧，返回 1，否则返回 2（相交）
    return true;
}


#endif //HELLO_MAC_FRUSTUM_H
