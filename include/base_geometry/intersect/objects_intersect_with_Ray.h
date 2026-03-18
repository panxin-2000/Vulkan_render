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
    return true;
}

inline bool intersect(const AABB_min_max<Point_2> &L_box, const Ray<Point_2> &ray) {
    const auto ray_y_1 = ray.point.y + ray.direction.y / ray.direction.x * (L_box.min_point.x - ray.point.x);
    const auto ray_y_2 = ray.point.y + ray.direction.y / ray.direction.x * (L_box.max_point.x - ray.point.x);
    auto bool_1        = have_intersect_axis(ray_y_1, ray_y_2, L_box.min_point.y, L_box.max_point.y);
    const auto ray_x_1 = ray.point.x + ray.direction.x / ray.direction.y * (L_box.min_point.y - ray.point.y);
    const auto ray_x_2 = ray.point.x + ray.direction.x / ray.direction.y * (L_box.max_point.y - ray.point.y);
    auto bool_2        = have_intersect_axis(ray_x_1, ray_x_2, L_box.min_point.x, L_box.max_point.x);
    if (bool_1 && bool_2) {
        auto bool_3 = (ray_x_1 - ray.point.x) * ray.direction.x >= 0;
        auto bool_4 = (ray_x_2 - ray.point.x) * ray.direction.x >= 0;
        auto bool_5 = (ray_y_1 - ray.point.y) * ray.direction.y >= 0;
        auto bool_6 = (ray_y_2 - ray.point.y) * ray.direction.y >= 0;
        if ((bool_3 || bool_4) && (bool_5 || bool_6)) {
            // bool_3 或 bool_4 必须有一个为真  bool_5 或 bool_6 必须有一个为真
            return true;
        }
        return false;
    }
    return true;
}

inline bool intersect(const AABB_min_max<Point_3> &L_box, const Ray<Point_3> &ray) {
}


template<typename T>
bool intersect(const Sphere<T> &sphere, const Ray<T> &ray) {
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

#endif //HELLO_MAC_OBJECTS_INTERSECT_WITH_RAY_H
