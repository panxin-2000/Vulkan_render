//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_INTERSECTION_H
#define HELLO_MAC_INTERSECTION_H
#include "../geometry/Trapezoid.h"
#include "../geometry/triangle.h"
#include "../geometry/AABB_bounding_box.h"
#include "../geometry/segment.h"
#include "base_element/point_2.h"

template<typename T>
bool intersect(const AABB_min_max<T> &box, const T &test_point) {
    if (box.min_point <= test_point && test_point <= box.max_point)
        return true;
    return false;
}

template<typename T>
bool intersect(const AABB_centroid<T> &box, const T &test_point) {
    if (box.centroid_point - box.direction_interval <= test_point &&
        test_point <= box.centroid_point + box.direction_interval)
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
    if (Point_2::is_anticlockwise(triangle.a, triangle.b, test_point) != Point_2::anticlockwise::clockwise &&
        Point_2::is_anticlockwise(triangle.b, triangle.c, test_point) != Point_2::anticlockwise::clockwise &&
        Point_2::is_anticlockwise(triangle.c, triangle.a, test_point) != Point_2::anticlockwise::clockwise) {
        return true;
    }
    return false;
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
    if (Point_2::is_anticlockwise(C_point, D_point, test_point) != Point_2::anticlockwise::clockwise &&
        Point_2::is_anticlockwise(D_point, B_point, test_point) != Point_2::anticlockwise::clockwise &&
        Point_2::is_anticlockwise(B_point, A_point, test_point) != Point_2::anticlockwise::clockwise &&
        Point_2::is_anticlockwise(A_point, C_point, test_point) != Point_2::anticlockwise::clockwise) {
        return true;
    }
    return false;
}



#endif //HELLO_MAC_INTERSECTION_H
