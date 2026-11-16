//
// Created by 潘鑫 on 2026/8/13.
//

#ifndef HELLO_MAC_AABB_BOX_H
#define HELLO_MAC_AABB_BOX_H

#include <Eigen/Eigen>
#include <utility>

class alignas(16) Render_AABB;
class alignas(16) Render_AABB_min;

class alignas(16) Render_AABB_min {
public:
    Eigen::Vector4f min;
    Eigen::Vector4f max;

    Render_AABB get_centroid() const;
};

class alignas(16) Render_AABB {
public:
    Eigen::Vector4f centroid_points_;
    Eigen::Vector4f direction_intervals_;

    Render_AABB(Eigen::Vector4f centroid_points_,
                Eigen::Vector4f direction_intervals_) : centroid_points_(std::move(centroid_points_)),
                                                        direction_intervals_(std::move(direction_intervals_)) {
    }

    Render_AABB() = default;

    [[nodiscard]] Render_AABB_min get_aabb_min() const;
};

inline Render_AABB Render_AABB_min::get_centroid() const {
    return Render_AABB{
        (min + max) * 0.5f,
        (max - min) * 0.5f
    };
}

inline Render_AABB_min Render_AABB::get_aabb_min() const {
    return Render_AABB_min{
        centroid_points_ - direction_intervals_,
        centroid_points_ + direction_intervals_
    };
}


template<typename T>
inline T merge_AABBs(const std::vector<T> &aabbs) {
    Eigen::Vector4f combined_min = Eigen::Vector4f::Constant(std::numeric_limits<float>::infinity());
    Eigen::Vector4f combined_max = Eigen::Vector4f::Constant(-std::numeric_limits<float>::infinity());

    for (const auto &aabb: aabbs) {
        Eigen::Vector4f p_min = aabb.centroid_points_ - aabb.direction_intervals_;
        Eigen::Vector4f p_max = aabb.centroid_points_ + aabb.direction_intervals_;

        // cwiseMin 和 cwiseMax 是 Eigen 的逐分量比较函数
        combined_min = combined_min.cwiseMin(p_min);
        combined_max = combined_max.cwiseMax(p_max);
    }
    return T(
             (combined_min + combined_max) * 0.5f,
             (combined_max - combined_min) * 0.5f
            );
}


class World_Space_AABB; // 前置声明

class Local_Space_AABB : public Render_AABB {
public:
    Local_Space_AABB() = default;

    // 从公共基类构造
    explicit Local_Space_AABB(const Render_AABB &base) : Render_AABB(base) {
    }

    // 从 World_Space_AABB 隐式/显式转换
    explicit Local_Space_AABB(const World_Space_AABB &other);
};

class World_Space_AABB : public Render_AABB {
public:
    World_Space_AABB() = default;

    World_Space_AABB(Eigen::Vector4f centroid_points_,
                     Eigen::Vector4f direction_intervals_) : Render_AABB({centroid_points_, direction_intervals_}) {
    }


    explicit World_Space_AABB(const Render_AABB &base) : Render_AABB(base) {
    }

    explicit World_Space_AABB(const Local_Space_AABB &other) : Render_AABB(other) {
    }
};

// 补齐 Local_Space_AABB 的实现
inline Local_Space_AABB::Local_Space_AABB(const World_Space_AABB &other) : Render_AABB(other) {
}


#endif //HELLO_MAC_AABB_BOX_H
