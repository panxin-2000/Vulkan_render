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
    const auto t0      = (L_box.min_point_ - ray.point) / ray.direction;
    const auto t1      = (L_box.max_point_ - ray.point) / ray.direction;
    const auto tmin3   = Point_2{std::min(t0.x, t1.x), std::min(t0.y, t1.y)};
    const auto tmax3   = Point_2{std::max(t0.x, t1.x), std::max(t0.y, t1.y)};
    const float t_near = std::max(tmin3.x, tmin3.y);
    const float t_far  = std::min(tmax3.x, tmax3.y);
    const bool hit     = t_near <= t_far;
    return hit && (t_near > 0 || t_far > 0);
}

inline bool is_intersect(const AABB_min_max<Point_3> &L_box, const Ray<Point_3> &ray) {
    const auto t0      = (L_box.min_point_ - ray.point) / ray.direction;
    const auto t1      = (L_box.max_point_ - ray.point) / ray.direction;
    const auto tmin3   = Point_3{std::min(t0.x, t1.x), std::min(t0.y, t1.y), std::min(t0.z, t1.z)};
    const auto tmax3   = Point_3{std::max(t0.x, t1.x), std::max(t0.y, t1.y), std::max(t0.z, t1.z)};
    const float t_near = std::max(tmin3.x, std::max(tmin3.y, tmin3.z));
    const float t_far  = std::min(tmax3.x, std::min(tmax3.y, tmax3.z));
    const bool hit     = t_near <= t_far;
    return hit && (t_near > 0 || t_far > 0);
}

inline bool is_intersect(const AABB_min_max<Point_3> &L_box, const Ray<Point_3> &ray, float &t_min, float &t_max) {
    // pnanovdb_vec3_t dir_inv = pnanovdb_vec3_div(pnanovdb_vec3_uniform(1.f), PNANOVDB_DEREF(direction));
    // pnanovdb_vec3_t t0      = pnanovdb_vec3_mul(pnanovdb_vec3_sub(PNANOVDB_DEREF(bbox_min), PNANOVDB_DEREF(origin)),
    //                                        dir_inv);
    // pnanovdb_vec3_t t1 = pnanovdb_vec3_mul(pnanovdb_vec3_sub(PNANOVDB_DEREF(bbox_max), PNANOVDB_DEREF(origin)),
    //                                        dir_inv);
    // pnanovdb_vec3_t tmin3 = pnanovdb_vec3_min(t0, t1);
    // pnanovdb_vec3_t tmax3 = pnanovdb_vec3_max(t0, t1);
    // float tnear           = pnanovdb_max(tmin3.x, pnanovdb_max(tmin3.y, tmin3.z));
    // float tfar            = pnanovdb_min(tmax3.x, pnanovdb_min(tmax3.y, tmax3.z));
    // pnanovdb_bool_t hit   = tnear <= tfar;
    // PNANOVDB_DEREF(tmin)  = pnanovdb_max(PNANOVDB_DEREF(tmin), tnear);
    // PNANOVDB_DEREF(tmax)  = pnanovdb_min(PNANOVDB_DEREF(tmax), tfar);
    // return hit;

    const auto t0      = (L_box.min_point_ - ray.point) / ray.direction;
    const auto t1      = (L_box.max_point_ - ray.point) / ray.direction;
    const auto tmin3   = Point_3{std::min(t0.x, t1.x), std::min(t0.y, t1.y), std::min(t0.z, t1.z)};
    const auto tmax3   = Point_3{std::max(t0.x, t1.x), std::max(t0.y, t1.y), std::max(t0.z, t1.z)};
    const float t_near = std::max(tmin3.x, std::max(tmin3.y, tmin3.z));
    const float t_far  = std::min(tmax3.x, std::min(tmax3.y, tmax3.z));
    const bool hit     = t_near <= t_far;
    t_min              = std::max(t_min, t_near);
    t_max              = std::min(t_max, t_far);
    return hit;
}


template<typename T>
bool is_intersect(const Sphere<T> &sphere, const Ray<T> &ray) {
    // 与球相交与判断结果之间是存在一个优化的办法的
    // 在光线追踪的最简实现中看到过 smallpt 这里比它多判断了一个条件
    // 优化了一元二次方程
    auto center_to_ray_start = ray.point - sphere.center;
    auto c                   = ((center_to_ray_start.dot(center_to_ray_start)) - sphere.radius * sphere.radius);
    if (c < 0) {
        // 此时光线发射点在 球中
        // 如果光线的渲染要返回false
        // 体积雾的话又是true
        return true;
    }
    auto direction  = ray.direction;
    auto b_half     = (center_to_ray_start.dot(direction));
    auto a          = (direction.dot(direction));
    auto delta_half = b_half * b_half - (direction.dot(direction)) * c;
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
