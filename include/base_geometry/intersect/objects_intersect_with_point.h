//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_INTERSECTION_H
#define HELLO_MAC_INTERSECTION_H
#include "base_geometry/base.h"


template<typename T>
bool intersect(const AABB_min_max<T> &box, const T &test_point) {
    if (box.min_point_ <= test_point && test_point <= box.max_point_)
        return true;
    return false;
}

template<typename T>
bool intersect(const AABB_centroid<T> &box, const T &test_point) {
    if (box.centroid_point_ - box.direction_interval_ <= test_point &&
        test_point <= box.centroid_point_ + box.direction_interval_)
        return true;
    return false;
}

/**
 * 测试点是否在三角形内，三角形的三个点没有顺序要求
 * @tparam T
 * @param triangle
 * @param test_point
 * @return
 */
template<typename T>
bool intersect(const Triangle<T> &triangle, const T &test_point) {
    auto temp1 = Point_2::is_anticlockwise(triangle.a, triangle.b, test_point);
    auto temp2 = Point_2::is_anticlockwise(triangle.b, triangle.c, test_point);
    auto temp3 = Point_2::is_anticlockwise(triangle.c, triangle.a, test_point);
    if (((temp1 | temp2 | temp3) == Point_2::anticlockwise::collinear_and_clockwise) ||
        ((temp1 | temp2 | temp3) == Point_2::anticlockwise::collinear_and_counterclockwise) ||
        ((temp1 | temp2 | temp3) == Point_2::anticlockwise::clockwise) ||
        ((temp1 | temp2 | temp3) == Point_2::anticlockwise::counterclockwise)) {
        return true;
    }
    return false;

    // 下面的逻辑应该是和上面的一样的
    if (((temp1 | temp2 | temp3) == Point_2::anticlockwise::clockwise_and_counterclockwise) ||
        ((temp1 | temp2 | temp3) == Point_2::anticlockwise::collinear_and_clock_and_counter)) {
        return false;
    }
    return true;
}

/**
 * 判断相交，点在边上也算是相交
 * @tparam T
 * @param trapezoid
 * @param test_point
 * @return
 */
template<typename T>
inline bool intersect(const Trapezoid &trapezoid, const T &test_point) {
    auto A_point = trapezoid.left_upper;
    auto B_point = trapezoid.right_upper;
    auto C_point = trapezoid.left_lower;
    auto D_point = trapezoid.right_lower;
    auto bool_1  = Point_2::is_anticlockwise(C_point, D_point, test_point);
    auto bool_2  = Point_2::is_anticlockwise(D_point, B_point, test_point);
    auto bool_3  = Point_2::is_anticlockwise(B_point, A_point, test_point);
    auto bool_4  = Point_2::is_anticlockwise(A_point, C_point, test_point);
    if (((bool_1 | bool_2 | bool_3 | bool_4) != Point_2::anticlockwise::clockwise)) {
        // 只有单一的一种必然是不相交的
        return true;
    }
    return false;
}

template<typename T>
inline bool intersect(const Sphere<T> &sphere, const T &test_point) {
    if (dot((test_point - sphere.center), (test_point - sphere.center)) <=
        (sphere.radius * sphere.radius)) {
        return true;
    }
    return false;
}

template<typename T>
inline bool intersect(const Plane<T> &plane, const T &test_point) {
    if (abs(dot((test_point - plane.point), (plane.normal))) < 0.0000001) {
        return true;
    }
    return false;
}

template<typename T>
inline float distance_of_box_center(const AABB_centroid<T> &box, const T &test_point) {
    T distance = box.centroid_point_ - test_point;
    return dot(distance, distance);
}

template<typename T>
inline float distance_of_box_center(const AABB_min_max<T> &box, const T &test_point) {
    T distance = (box.max_point_ + box.min_point_) / 2 - test_point;
    return dot(distance, distance);
}


#endif //HELLO_MAC_INTERSECTION_H
