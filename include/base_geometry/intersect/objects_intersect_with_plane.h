//
// Created by 潘鑫 on 2025/12/7.
//

#ifndef HELLO_MAC_OBJECTS_INTERSECT_WITH_PLANE_H
#define HELLO_MAC_OBJECTS_INTERSECT_WITH_PLANE_H
#include "base_geometry/base.h"



template<typename T>
inline bool intersect(const Plane<T> &plane, const Segment<T> &segment) {
    // 一个点在平面一侧，另一个点在平面另一侧
    auto start_distance = plane.distance(segment.start_point);
    auto end_distance   = plane.distance(segment.end_point);
    if (end_distance * start_distance <= 0) {
        return true;
    }
    return false;
}

template<typename T>
inline bool intersect(const Plane<T> &plane, const Ray<T> &ray) {
    // 直线的方向与法线 不垂直时 且 射线的方向是 指向平面的
    // 第二个条件可以被理解为，求出与平面的交点，并判断t的方向是正是负
    // 只是为了求出t,交点是可以假设而不求出的
    // 上面还是麻烦了，判断光线起点在哪一侧
    // 平面的标记点，加上光线的方向，得到另一个点
    // 判断这个点是否和光线的起点在同一侧，在的话，就相交，否则不相交
    auto start_distance = plane.distance(ray.start_point);
    auto end_distance   = plane.distance(plane.point + ray.direction);
    if (end_distance * start_distance <= 0) {
        return true;
    }
    return false;
}

template<typename T>
inline bool intersect(const Plane<T> &plane, const Straight_line<T> &straight_line) {
    // 直线怎么判断？ // 直线的方向与法线 不垂直时 永远相交
    if (abs(dot((straight_line.direction), (plane.normal))) < 0.0000001) {
        return false;
    }
    // 垂直时，点在平面上才相交
    if (abs(plane.distance(straight_line.point)) < 0.0000001) {
        return true;
    }
    return false;
}


template<typename T>
inline bool intersect(const Plane<T> &plane, const Triangle<T> &triangle) {
    // 三角形中，任意一个点在平面一侧，另一个点在平面另一侧
    auto a_distance = plane.distance(triangle.a);
    auto b_distance = plane.distance(triangle.b);
    auto c_distance = plane.distance(triangle.c);
    if (a_distance * b_distance <= 0 ||
        b_distance * c_distance <= 0 ||
        c_distance * a_distance <= 0) {
        return true;
    }
    return false;
}

template<typename T>
inline bool intersect(const Plane<T> &plane, const AABB_min_max<T> &box) {
    // 确实是很有想法的一个解法
    return false;
}
#endif //HELLO_MAC_OBJECTS_INTERSECT_WITH_PLANE_H
