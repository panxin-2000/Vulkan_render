//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_OBJECTS_INTERSECT_WITH_SPHERE_H
#define HELLO_MAC_OBJECTS_INTERSECT_WITH_SPHERE_H
#include "base_element/geometry/segment.h"
#include "base_element/geometry/sphere_bounding_volume.h"
#include "base_element/geometry/triangle.h"
#include "base_element/intersect/objects_intersect_with_segment.h"

template<typename T>
bool intersect(const Sphere<T> &L_sphere, const Triangle<T> &triangle) {
    // 有更优的方案，写起来稍微麻烦一点
    if (intersect(L_sphere, Segment<T>{triangle.a, triangle.b})) {
        return true;
    }
    if (intersect(L_sphere, Segment<T>{triangle.b, triangle.c})) {
        return true;
    }
    if (intersect(L_sphere, Segment<T>{triangle.c, triangle.a})) {
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
bool intersect(const Sphere<T> &L_sphere, const Sphere<T> &R_sphere) {
    auto distance = L_sphere.center - R_sphere.center;
    auto distanceSquared = dot(distance, distance);
    if (distanceSquared <= L_sphere.radius * L_sphere.radius + R_sphere.radius * R_sphere.radius) {
        return true;
    }
    return false;
}

// 获取相交的结果的时候，其实就不太好表示了
// 两个圆相交之后的结果也是一个圆，或者一个点
// 或者拿到的结果是相交之后体积， 怎么表示这个圆呢？


#endif //HELLO_MAC_OBJECTS_INTERSECT_WITH_SPHERE_H
