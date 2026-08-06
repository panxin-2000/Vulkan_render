//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_INTERSECTION_H
#define HELLO_MAC_INTERSECTION_H
#include "base_geometry/base.h"

/**
 * 返回的值是距离的平方
 * @tparam T
 * @param segment
 * @param test_point
 * @return
 */
template<typename T>
float distance(const Segment<T> &segment, const T &test_point) {
    auto direction = segment.end_point - segment.start_point;
    auto PA        = test_point - segment.start_point;
    auto t         = (PA.dot(direction)) / (direction.dot(direction));
    if (t > 0 && t < 1) {
        auto D        = segment.start_point + direction * t;
        auto distance = ((test_point - D).dot(test_point - D));
        return distance;
    } else if (t <= 0) {
        auto distance = ((test_point - segment.start_point).dot(test_point - segment.start_point));
        return distance;
    } else if (t >= 0) {
        auto distance = ((test_point - segment.end_point).dot(test_point - segment.end_point));
        return distance;
    }
    return NAN;
}

template<typename T>
float distance(const T &point_L, const T &point_R) {
    return ((point_L - point_R).dot(point_L - point_R));
}

template<typename T>
float distance(const Ray<T> &ray, const T &test_point) {
    auto direction = ray.direction;
    auto PA        = test_point - ray.point;
    auto t         = (PA.dot(direction)) / (direction.dot(direction));
    if (t > 0) {
        auto D        = ray.point + direction * t;
        auto distance = ((test_point - D).dot(test_point - D));
        return distance;
    } else if (t <= 0) {
        auto distance = ((test_point - ray.point).dot(test_point - ray.point));
        return distance;
    }
    return NAN;
}

template<typename T>
float distance(const Straight_line<T> &line, const T &test_point) {
    auto direction = line.direction;
    auto PA        = test_point - line.point;
    auto t         = (PA.dot(direction)) / (direction.dot(direction));
    auto D         = line.point + direction * t;
    auto distance  = ((test_point - D).dot(test_point - D));
    return distance;
}

template<typename T>
bool is_intersect(const AABB_min_max<T> &box, const T &test_point) {
    if (box.min_point_ <= test_point && test_point <= box.max_point_)
        return true;
    return false;
}

template<typename T>
bool is_intersect(const AABB_centroid<T> &box, const T &test_point) {
    if (box.centroid_point_ - box.direction_interval_ <= test_point &&
        test_point <= box.centroid_point_ + box.direction_interval_)
        return true;
    return false;
}

template<typename T>
inline float distance_of_box_center(const AABB_centroid<T> &box, const T &test_point) {
    T distance = box.centroid_point_ - test_point;
    return (distance.dot(distance));
}

template<typename T>
inline float distance_of_box_center(const AABB_min_max<T> &box, const T &test_point) {
    T distance = (box.max_point_ + box.min_point_) / 2 - test_point;
    return (distance.dot(distance));
}


template<typename T>
inline bool is_intersect(const Plane<T> &plane, const T &test_point) {
    if (abs(((test_point - plane.point).dot(plane.normal))) < 0.0000001) {
        return true;
    }
    return false;
}

template<typename T>
inline bool is_intersect(const Sphere<T> &sphere, const T &test_point) {
    if (((test_point - sphere.center).dot(test_point - sphere.center)) <=
        (sphere.radius * sphere.radius)) {
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
inline bool is_intersect(const Trapezoid &trapezoid, const T &test_point) {
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

/**
 * 测试点是否在三角形内，三角形的三个点没有顺序要求
 * @tparam T
 * @param triangle
 * @param test_point
 * @return
 */
template<typename T>
bool is_intersect(const Triangle<T> &triangle, const T &test_point) {
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

    // 另一种表示方式，
    // T a2b       = triangle.b - triangle.a;
    // T a2c       = triangle.c - triangle.a;
    // auto a2p    = test_point - triangle.a;
    // float area  = a2b.single_area(a2c);
    // float alpha = a2p.single_area(a2c) / area;
    // float beta  = a2b.single_area(a2p) / area;
    // float gamma = 1.0f - (alpha + beta);
    // if (alpha < 0.0f || beta < 0.0f || gamma < 0.0f) {
    //     return false;
    // }
    // return true;


    // 下面的逻辑应该是和上面的一样的
    if (((temp1 | temp2 | temp3) == Point_2::anticlockwise::clockwise_and_counterclockwise) ||
        ((temp1 | temp2 | temp3) == Point_2::anticlockwise::collinear_and_clock_and_counter)) {
        return false;
    }
    return true;
}

#endif //HELLO_MAC_INTERSECTION_H
