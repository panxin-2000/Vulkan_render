//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_OBJECTS_INTERSECT_WITH_SEGMENT_H
#define HELLO_MAC_OBJECTS_INTERSECT_WITH_SEGMENT_H
#include "../geometry/segment.h"
#include "../point_2.h"
#include "../geometry/AABB_bounding_box.h"
#include "objects_intersect_with_point.h"
#include "base_element/intersect/objects_intersect_with_AABB.h"

inline bool intersect(const Segment<Point_2> &L_segment, const Segment<Point_2> &R_segment) {
    AABB_min_max<Point_2> L_AABB{L_segment.start_point, L_segment.end_point};
    AABB_min_max<Point_2> R_AABB{R_segment.start_point, R_segment.end_point};
    if (!intersect(L_AABB, R_AABB)) {
        return false;
    }

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


template<typename T>
bool intersect(const AABB_centroid<T> &L_box, const Segment<T> &R_segment) {
    AABB_min_max<Point_2> R_AABB{R_segment.start_point, R_segment.end_point};
    if (!intersect(L_box, R_AABB)) {
        return false;
    }
    return true;

    return false;
}


template<typename Point_2>
bool find_axis_aligned_four_point(const AABB_min_max<Point_2> &L_box,
                                  const Segment<Point_2> &R_segment,
                                  std::vector<Point_2> *result) {
    auto offset = R_segment.end_point - R_segment.start_point;
    float k_x = offset.y / offset.x;
    float k_y = offset.x / offset.y;
    float interval_to_min_x = L_box.min_point.x - R_segment.start_point.x;
    float interval_to_max_x = L_box.max_point.x - R_segment.start_point.x;
    float interval_to_min_y = L_box.min_point.y - R_segment.start_point.y;
    float interval_to_max_y = L_box.max_point.y - R_segment.start_point.y;
    float segment_min_x = std::min(R_segment.start_point.x, R_segment.end_point.x);
    float segment_max_x = std::max(R_segment.start_point.x, R_segment.end_point.x);
    float segment_min_y = std::min(R_segment.start_point.y, R_segment.end_point.y);
    float segment_max_y = std::max(R_segment.start_point.y, R_segment.end_point.y);
    // result_1 = {L_box.min_point.x, R_segment.start_point.y + k_x * start_point_to_min_x};
    // result_2 = {L_box.max_point.x, R_segment.start_point.y + k_x * start_point_to_max_x};
    // result_3 = std::min(R_segment.start_point.y, R_segment.end_point.y);
    // result_4 = {R_segment.start_point.x + k_y * start_point_to_max_y, L_box.max_point.y};
    // 判断有点多，不知道能不能省一点内容
    auto number = 0;
    auto a_y = R_segment.start_point.y + k_x * interval_to_min_x;
    auto a_x = R_segment.start_point.x + interval_to_min_x;
    if (a_y >= L_box.min_point.y && L_box.max_point.y >= a_y &&
        a_x >= segment_min_x &&
        segment_max_x <= a_x
    ) {
        result->push_back({L_box.min_point.x, a_y});
        number++;
    }
    auto c_y = R_segment.start_point.y + k_x * interval_to_max_x;
    auto c_x = R_segment.start_point.x + interval_to_max_x;
    if (c_y >= L_box.min_point.y && L_box.max_point.y >= c_y &&
        c_x >= segment_min_x &&
        segment_max_x <= c_x) {
        result->push_back({L_box.max_point.x, c_y});
        number++;
    }
    auto b_x = R_segment.start_point.x + k_y * interval_to_min_y;
    auto b_y = R_segment.start_point.y + k_y * interval_to_min_y;
    if (b_x >= L_box.min_point.x && L_box.max_point.x >= b_x &&
        b_y >= segment_min_y &&
        segment_max_y <= b_y) {
        result->push_back({b_x, L_box.min_point.y});
        number++;
    }
    auto d_x = R_segment.start_point.x + k_y * interval_to_max_y;
    auto d_y = R_segment.start_point.y + k_y * interval_to_max_y;
    if (d_x >= L_box.min_point.x && L_box.max_point.x >= d_x &&
        d_y >= segment_min_y &&
        segment_max_y <= d_y) {
        result->push_back({d_x, L_box.max_point.y});
        number++;
    }
    if (number > 0)
        return true;
    return false;
}

// 之后还需要两个函数，返回的点是否在线段上，是否在光线上
// 其实在上面也是能够判断完成的

inline bool intersect(const AABB_min_max<Point_2> &L_box, const Segment<Point_2> &R_segment) {
    // 判断两个包围盒是否存在相交
    if (intersect(L_box, AABB_min_max<Point_2>(R_segment.start_point, R_segment.end_point))) {
        Point_2 box_min_x_min_y = {L_box.min_point.x, L_box.min_point.y};
        Point_2 box_min_x_max_y = {L_box.min_point.x, L_box.max_point.y};
        Point_2 box_mam_x_min_y = {L_box.max_point.x, L_box.min_point.y};
        Point_2 box_max_x_max_y = {L_box.max_point.x, L_box.max_point.y};
        auto bool_1 = Point_2::is_anticlockwise(R_segment.start_point, R_segment.end_point, box_min_x_min_y);
        auto bool_2 = Point_2::is_anticlockwise(R_segment.start_point, R_segment.end_point, box_min_x_max_y);
        auto bool_3 = Point_2::is_anticlockwise(R_segment.start_point, R_segment.end_point, box_mam_x_min_y);
        auto bool_4 = Point_2::is_anticlockwise(R_segment.start_point, R_segment.end_point, box_max_x_max_y);
        if (((bool_1 | bool_2 | bool_3 | bool_4) == Point_2::anticlockwise::counterclockwise) ||
            ((bool_1 | bool_2 | bool_3 | bool_4) == Point_2::anticlockwise::clockwise)) {
            // 只有单一的一种必然是不相交的
            return false;
        }
        return true;
    }
    return false;
}

template<typename T>
bool intersect_with_closest_result(const AABB_min_max<T> &L_box, const Segment<T> &R_segment, const T *result) {
    // 求最近点时，有一个稍微简单一点点办法，比较偏向于直线了
    // 找到最近点两个 # 点
    // 利用相似三角形可以比较快的得出结果
}


template<typename T>
bool intersect(const Sphere<T> &sphere, const Segment<T> &R_segment) {
    // 与球相交与判断结果之间是存在一个优化的办法的
    // 在光线追踪的最简实现中看到过
    // 优化了一元二次方程
}

#endif //HELLO_MAC_OBJECTS_INTERSECT_WITH_SEGMENT_H
