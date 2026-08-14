//
// Created by 潘鑫 on 2026/8/13.
//

#ifndef HELLO_MAC_AABB_BOX_H
#define HELLO_MAC_AABB_BOX_H

#include <Eigen/Eigen>

struct alignas(16) Render_AABB;
struct alignas(16) Render_AABB_min;

struct alignas(16) Render_AABB_min {
    Eigen::Vector4f min;
    Eigen::Vector4f max;

    Render_AABB get_centroid() const;
};

struct alignas(16) Render_AABB {
    Eigen::Vector4f centroid_points;
    Eigen::Vector4f direction_intervals;

    Render_AABB_min get_aabb_min() const;
};

inline Render_AABB Render_AABB_min::get_centroid() const {
    return Render_AABB{
        (min + max) * 0.5f,
        (max - min) * 0.5f
    };
}

inline Render_AABB_min Render_AABB::get_aabb_min() const {
    return Render_AABB_min{
        centroid_points - direction_intervals,
        centroid_points + direction_intervals
    };
}



inline Render_AABB merge_AABBs(const std::vector<Render_AABB> &aabbs) {
    Eigen::Vector4f combined_min = Eigen::Vector4f::Constant(std::numeric_limits<float>::infinity());
    Eigen::Vector4f combined_max = Eigen::Vector4f::Constant(-std::numeric_limits<float>::infinity());

    for (const auto &aabb: aabbs) {
        Eigen::Vector4f p_min = aabb.centroid_points - aabb.direction_intervals;
        Eigen::Vector4f p_max = aabb.centroid_points + aabb.direction_intervals;

        // cwiseMin 和 cwiseMax 是 Eigen 的逐分量比较函数
        combined_min = combined_min.cwiseMin(p_min);
        combined_max = combined_max.cwiseMax(p_max);
    }
    return {
        (combined_min + combined_max) * 0.5f,
        (combined_max - combined_min) * 0.5f
    };
}


#endif //HELLO_MAC_AABB_BOX_H
