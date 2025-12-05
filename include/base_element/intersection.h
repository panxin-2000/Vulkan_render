//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_INTERSECTION_H
#define HELLO_MAC_INTERSECTION_H
#include "Trapezoid.h"
#include "triangle.h"
#include "base_element/AABB_bounding_box.h"
#include "base_element/segment.h"
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

inline bool intersect(Segment<Point_2> &L_segment, Segment<Point_2> &R_segment) {
    Point_2 ab = L_segment.end_point - L_segment.start_point;
    Point_2 ac = R_segment.start_point - L_segment.start_point;
    Point_2 ad = R_segment.end_point - L_segment.start_point;

    Point_2 cd = R_segment.end_point - R_segment.start_point;
    Point_2 ca = L_segment.start_point - R_segment.start_point;
    Point_2 cb = L_segment.end_point - R_segment.start_point;

    // ac ad 在 ab 的 不同侧的边 且  ca cb 在 cd 的不同侧的边
    float f1 = ab.single_area(ac);
    float f2 = ab.single_area(ad);
    float f3 = cd.single_area(ca);
    float f4 = cd.single_area(cb);
    if (f1 * f2 < 0 && f3 * f4 < 0) {
        // 这个应该是一个比较简单的判断了 // 算法导论上的比较符号太多了
        // 这里似乎是有问题的，之前写的有问题，之前的符号写的有问题
        return true;
    }
    // 如果有任何一个等于零的时候，那么需要判断是否在线上，因为不在线上也可能为零
    // 其实这里并不是很准确，因为应该判断小于一个固定小的常数。
    if (f1 == 0 && intersect(AABB_min_max<Point_2>{L_segment.start_point, L_segment.end_point},
                             R_segment.start_point))
        return true;
    if (f2 == 0 && intersect(AABB_min_max<Point_2>{L_segment.start_point, L_segment.end_point},
                             R_segment.end_point))
        return true;
    if (f3 == 0 && intersect(AABB_min_max<Point_2>{R_segment.start_point, R_segment.end_point},
                             L_segment.start_point))
        return true;
    if (f4 == 0 && intersect(AABB_min_max<Point_2>{R_segment.start_point, R_segment.end_point},
                             L_segment.end_point))
        return true;
    return false;
}


#endif //HELLO_MAC_INTERSECTION_H
