//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_OBJECTS_INTERSECT_WITH_SPHERE_H
#define HELLO_MAC_OBJECTS_INTERSECT_WITH_SPHERE_H
#include "base_geometry/base.h"

#include "objects_intersect_with_segment.h"

template<typename T>
bool is_intersect(const Sphere<T> &L_sphere, const Triangle<T> &triangle) {
    // 有更优的方案，写起来稍微麻烦一点
    // 不对，如果球在三角形内部呢？
    if (is_intersect(L_sphere, Segment<T>{triangle.a, triangle.b})) {
        return true;
    }
    if (is_intersect(L_sphere, Segment<T>{triangle.b, triangle.c})) {
        return true;
    }
    if (is_intersect(L_sphere, Segment<T>{triangle.c, triangle.a})) {
        return true;
    }
    return false;
}

/**
 * 两个圆心的距离小于半径距离之和则相交
 * @tparam T
 * @param L_sphere
 * @param R_sphere
 * @return
 */
template<typename T>
bool is_intersect(const Sphere<T> &L_sphere, const Sphere<T> &R_sphere) {
    auto distance        = L_sphere.center - R_sphere.center;
    auto distanceSquared = (distance.dot(distance));
    if (distanceSquared <= L_sphere.radius * L_sphere.radius + R_sphere.radius * R_sphere.radius) {
        return true;
    }
    return false;
}

// 获取相交的结果的时候，其实就不太好表示了
// 两个圆相交之后的结果也是一个圆，或者一个点
// 或者拿到的结果是相交之后体积， 怎么表示这个圆呢？

template<typename T>
float distance(const Sphere<T> &sphere, const T &test_point) {
    auto distance        = ((sphere.center - test_point).dot(sphere.center - test_point));
    auto distanceSquared = sphere.radius * sphere.radius;
    if (distance <= distanceSquared) {
        return 0;
    } else {
        // 点到圆心的距离，再减去半径，得到点到圆的距离，之后再平方
        return (sqrt(distance) - sphere.radius) * (sqrt(distance) - sphere.radius);
    }
}

#endif //HELLO_MAC_OBJECTS_INTERSECT_WITH_SPHERE_H
