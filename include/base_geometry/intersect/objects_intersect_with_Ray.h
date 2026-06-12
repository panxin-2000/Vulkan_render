//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_OBJECTS_INTERSECT_WITH_RAY_H
#define HELLO_MAC_OBJECTS_INTERSECT_WITH_RAY_H


#include "base_geometry/base.h"


// 一个射线是否与一个包围盒相交
// 然后是二维和三维的区别
// 先去看二维的内容
// 二维的时候需要如何做
// 找到射线 在 包围盒的 x 区间 与 y 区间的线段
// 判定线段 是否与 包围盒 相交
// 然后是方向，射线是有方向的

inline bool have_intersect_axis(const float x1, const float x2, const float x3, const float x4) {
    //    x1-------------------x2
    //                               x3------------------x4
    //           x_12_half_distance
    //               ----------       ----------
    //                                 x_34_half_distance
    //               *************************** abs(x_12_middle - x_34_middle)
    const auto x_12_middle        = (x1 + x2) * 0.5;
    const auto x_34_middle        = (x3 + x4) * 0.5;
    const auto x_12_half_distance = std::abs((x1 - x2) * 0.5);
    const auto x_34_half_distance = std::abs((x3 - x4) * 0.5);
    if (abs(x_12_middle - x_34_middle) > x_12_half_distance + x_34_half_distance) {
        return false;
    }
    return true; // 这里是 比较逻辑 导致了 nan 返回的结果为 true 很巧合的一点 但是某些地方会出错，可能
}

inline bool is_intersect(const AABB_min_max<Point_2> &L_box, const Ray<Point_2> &ray) {
    auto t_min = (L_box.min_point_ - ray.point) / ray.direction;
    auto t_max = (L_box.max_point_ - ray.point) / ray.direction;
    if ((t_min.x >= 0 || t_max.x >= 0 || std::isnan(t_min.x) || std::isnan(t_min.x)) &&
        (t_min.y >= 0 || t_max.y >= 0 || std::isnan(t_min.y) || std::isnan(t_max.y))) {
        // 上面的判断是一个半平面的判断

        // 一个射线在 可能的x轴或y轴上 有四个交点，任何一个在包围盒中就是正确的
        // const auto a = ray.point + ray.direction * t_min.x;
        // const auto b = ray.point + ray.direction * t_max.x;
        // const auto c = ray.point + ray.direction * t_min.y;
        // const auto d = ray.point + ray.direction * t_max.y;
        // if (intersect(L_box, a) || intersect(L_box, b) || intersect(L_box, c) || intersect(L_box, d)) {
        //     return true;
        // }
        // return false;

        std::swap(t_min.x, t_min.y); // 为什么要交换？
        std::swap(t_max.x, t_max.y);
        const auto v_1    = ray.point + t_min * ray.direction;
        const auto v_2    = ray.point + t_max * ray.direction;
        const auto bool_1 = have_intersect_axis(v_1.y, v_2.y, L_box.min_point_.y, L_box.max_point_.y);
        const auto bool_2 = have_intersect_axis(v_1.x, v_2.x, L_box.min_point_.x, L_box.max_point_.x);
        if (bool_1 && bool_2) {
            return true;
        }
    }
    return false;
}

inline bool is_intersect(const AABB_min_max<Point_3> &L_box, const Ray<Point_3> &ray) {
    auto t_min = (L_box.min_point_ - ray.point) / ray.direction;
    auto t_max = (L_box.max_point_ - ray.point) / ray.direction;
    if ((t_min.x >= 0 || t_max.x >= 0 || std::isnan(t_min.x) || std::isnan(t_min.x)) &&
        (t_min.y >= 0 || t_max.y >= 0 || std::isnan(t_min.y) || std::isnan(t_min.y)) &&
        (t_min.z >= 0 || t_max.z >= 0 || std::isnan(t_min.z) || std::isnan(t_max.z))) {
        auto a = ray.point + ray.direction * t_min.x;
        auto b = ray.point + ray.direction * t_max.x;
        auto c = ray.point + ray.direction * t_min.y;
        auto d = ray.point + ray.direction * t_max.y;
        auto e = ray.point + ray.direction * t_min.z;
        auto f = ray.point + ray.direction * t_max.z;
        // 任何一个在就可以
        if (is_intersect(L_box, a) || is_intersect(L_box, b) || is_intersect(L_box, c) || is_intersect(L_box, d) ||
            is_intersect(L_box, e) || is_intersect(L_box, f)) {
            return true;
        }
        return false;
    }
    return false;
}


template<typename T>
bool is_intersect(const Sphere<T> &sphere, const Ray<T> &ray) {
    // 与球相交与判断结果之间是存在一个优化的办法的
    // 在光线追踪的最简实现中看到过 smallpt 这里比它多判断了一个条件
    // 优化了一元二次方程
    auto center_to_ray_start = ray.point - sphere.center;
    auto c                   = (dot(center_to_ray_start, center_to_ray_start) - sphere.radius * sphere.radius);
    if (c < 0) {
        // 此时光线发射点在 球中
        // 如果光线的渲染要返回false
        // 体积雾的话又是true
        return true;
    }
    auto direction  = ray.direction;
    auto b_half     = dot(center_to_ray_start, direction);
    auto a          = dot(direction, direction);
    auto delta_half = b_half * b_half - dot(direction, direction) * c;
    if (delta_half < 0) {
        return false;
    }
    if (-b_half < 0) {
        // x_1 + x_2 = -b/a
        // 此时不在球中，要么都是正，要么都负
        // 都是负时，-b < 0 , 因为 a 一直为正
        return false;
    }

    return true;
}




template<typename T>
bool is_intersect(const Triangle<T> &triangle, const Ray<T> &ray) {
    return false;
}

#endif //HELLO_MAC_OBJECTS_INTERSECT_WITH_RAY_H
