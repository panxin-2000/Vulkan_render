//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_OBJECTS_INTERSECT_WITH_SEGMENT_H
#define HELLO_MAC_OBJECTS_INTERSECT_WITH_SEGMENT_H
#include "base_geometry/base.h"

#include "objects_intersect_with_point.h"
#include "objects_intersect_with_AABB.h"


/**
 * 已知相交，求交点
 * @param L_box
 * @param segment
 * @param result
 * @return
 */
inline bool find_axis_aligned_four_point(const AABB_min_max<Point_2> &L_box,
                                         const Segment<Point_2> &segment,
                                         std::vector<Point_2> *result) {
    auto offset             = segment.end_point - segment.start_point;
    float k_x               = offset.y / offset.x;
    float k_y               = offset.x / offset.y;
    float interval_to_min_x = L_box.min_point_.x - segment.start_point.x;
    float interval_to_max_x = L_box.max_point_.x - segment.start_point.x;
    float interval_to_min_y = L_box.min_point_.y - segment.start_point.y;
    float interval_to_max_y = L_box.max_point_.y - segment.start_point.y;
    float segment_min_x     = std::min(segment.start_point.x, segment.end_point.x);
    float segment_max_x     = std::max(segment.start_point.x, segment.end_point.x);
    float segment_min_y     = std::min(segment.start_point.y, segment.end_point.y);
    float segment_max_y     = std::max(segment.start_point.y, segment.end_point.y);
    auto number             = 0;
    auto a_y                = segment.start_point.y + k_x * interval_to_min_x;
    auto a_x                = segment.start_point.x + interval_to_min_x;
    if (a_y >= L_box.min_point_.y && L_box.max_point_.y >= a_y &&
        a_x >= segment_min_x &&
        segment_max_x <= a_x
    ) {
        result->push_back({L_box.min_point_.x, a_y});
        number++;
    }
    auto c_y = segment.start_point.y + k_x * interval_to_max_x;
    auto c_x = segment.start_point.x + interval_to_max_x;
    if (c_y >= L_box.min_point_.y && L_box.max_point_.y >= c_y &&
        c_x >= segment_min_x &&
        segment_max_x <= c_x) {
        result->push_back({L_box.max_point_.x, c_y});
        number++;
    }
    auto b_x = segment.start_point.x + k_y * interval_to_min_y;
    auto b_y = segment.start_point.y + k_y * interval_to_min_y;
    if (b_x >= L_box.min_point_.x && L_box.max_point_.x >= b_x &&
        b_y >= segment_min_y &&
        segment_max_y <= b_y) {
        result->push_back({b_x, L_box.min_point_.y});
        number++;
    }
    auto d_x = segment.start_point.x + k_y * interval_to_max_y;
    auto d_y = segment.start_point.y + k_y * interval_to_max_y;
    if (d_x >= L_box.min_point_.x && L_box.max_point_.x >= d_x &&
        d_y >= segment_min_y &&
        segment_max_y <= d_y) {
        result->push_back({d_x, L_box.max_point_.y});
        number++;
    }
    if (number > 0)
        return true;
    return false;
}

// 之后还需要两个函数，返回的点是否在线段上，是否在光线上
// 其实在上面也是能够判断完成的


template<typename T>
inline bool is_intersect(const AABB_centroid<T> &L_box, const Segment<T> &segment) {
    return is_intersect(AABB_min_max<Point_2>(L_box), segment);
}

inline bool is_intersect(const AABB_min_max<Point_2> &L_box, const Segment<Point_2> &segment) {
    // 判断两个包围盒是否存在相交
    if (is_intersect(L_box, AABB_min_max<Point_2>(segment.start_point, segment.end_point))) {
        Point_2 box_min_x_min_y = {L_box.min_point_.x, L_box.min_point_.y};
        Point_2 box_min_x_max_y = {L_box.min_point_.x, L_box.max_point_.y};
        Point_2 box_mam_x_min_y = {L_box.max_point_.x, L_box.min_point_.y};
        Point_2 box_max_x_max_y = {L_box.max_point_.x, L_box.max_point_.y};
        auto bool_1             = Point_2::is_anticlockwise(segment.start_point, segment.end_point, box_min_x_min_y);
        auto bool_2             = Point_2::is_anticlockwise(segment.start_point, segment.end_point, box_min_x_max_y);
        auto bool_3             = Point_2::is_anticlockwise(segment.start_point, segment.end_point, box_mam_x_min_y);
        auto bool_4             = Point_2::is_anticlockwise(segment.start_point, segment.end_point, box_max_x_max_y);
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
bool intersect_with_closest_result(const AABB_min_max<T> &L_box, const Segment<T> &segment, const T *result) {
    // 求最近点时，有一个稍微简单一点点办法，比较偏向于直线了
    // 找到最近点两个 # 点
    // 利用相似三角形可以比较快的得出结果
}


template<typename T>
bool is_intersect(const Sphere<T> &sphere, const Segment<T> &segment) {
    Ray<T> ray_start(segment.start_point, segment.end_point - segment.start_point);
    auto center_to_segment_start = ray_start.point - sphere.center;
    auto c_start = ((center_to_segment_start.dot(center_to_segment_start)) - sphere.radius * sphere.radius);
    if (c_start < 0) {
        return true; // 起点在球中
    }
    Ray<T> ray_end(segment.end_point, segment.start_point - segment.end_point);
    auto center_to_segment_end = ray_end.point - sphere.center;
    auto c_end                 = ((center_to_segment_end.dot(center_to_segment_end)) - sphere.radius * sphere.radius);
    if (c_end < 0) {
        return true; // 钟点在球中
    }
    // 起点和终点都不在球中

    auto direction_start = ray_start.direction;
    auto b_half_start    = (center_to_segment_start.dot(direction_start));
    auto a               = (direction_start.dot(direction_start));
    auto delta_half      = b_half_start * b_half_start - (direction_start.dot(direction_start)) * c_start;
    if (delta_half < 0) {
        return false; // 这里决定了线段所在直线不会相交
    }
    auto direction_end = ray_end.direction;
    auto b_half_end    = (center_to_segment_end.dot(direction_end));
    if (-b_half_start < 0 || -b_half_end < 0) {
        return false; // 一个线段穿过球两次，所以不管那个点做起点，都不会小于零
    }
    return false; // 最后这一行没有添加具体的测试，看看什么情况下会到达这一行
}


inline bool is_intersect(const Trapezoid &trapezoid, const Segment<Point_2> &segment) {
    auto A_point = trapezoid.left_upper;
    auto B_point = trapezoid.right_upper;
    auto C_point = trapezoid.left_lower;
    auto D_point = trapezoid.right_lower;
    // 划分为两个三角形，之后再执行
    // auto bool_1 = Point_2::is_anticlockwise(C_point, D_point, test_point);
    // auto bool_2 = Point_2::is_anticlockwise(D_point, B_point, test_point);
    // auto bool_3 = Point_2::is_anticlockwise(B_point, A_point, test_point);
    // auto bool_4 = Point_2::is_anticlockwise(A_point, C_point, test_point);
    // if (((bool_1 | bool_2 | bool_3 | bool_4) != Point_2::anticlockwise::clockwise)) {
    // 只有单一的一种必然是不相交的
    // return true;
    // }
    return false;
}


inline bool intersect_pass_AABB(const Segment<Point_2> &L_segment, const Segment<Point_2> &segment) {
    const Point_2 ab = L_segment.end_point - L_segment.start_point;
    const Point_2 ac = segment.start_point - L_segment.start_point;
    const Point_2 ad = segment.end_point - L_segment.start_point;

    const Point_2 cd = segment.end_point - segment.start_point;
    const Point_2 ca = L_segment.start_point - segment.start_point;
    const Point_2 cb = L_segment.end_point - segment.start_point;

    // ac ad 在 ab 的 不同侧的边 且  ca cb 在 cd 的不同侧的边
    const float f1 = (ab.cross(ac));
    const float f2 = (ab.cross(ad));
    if (f1 * f2 > 0) {
        return false;
    }
    const float f3 = cd.cross( ca);
    const float f4 = cd.cross( cb);
    if (f3 * f4 > 0) {
        return false;
    }
    if (f1 * f2 < 0 && f3 * f4 < 0) {
        // 这个应该是一个比较简单的判断了 // 算法导论上的比较符号太多了
        // 这里似乎是有问题的，之前写的有问题，之前的符号写的有问题
        return true;
    }
    // 如果有任何一个等于零的时候，那么需要判断是否在线上，因为不在线上也可能为零
    // 其实这里并不是很准确，因为应该判断小于一个固定小的常数。
    else if (f1 == 0 && is_intersect(AABB_min_max<Point_2>{L_segment.start_point, L_segment.end_point},
                                     segment.start_point))
        return true;
    else if (f2 == 0 && is_intersect(AABB_min_max<Point_2>{L_segment.start_point, L_segment.end_point},
                                     segment.end_point))
        return true;
    else if (f3 == 0 && is_intersect(AABB_min_max<Point_2>{segment.start_point, segment.end_point},
                                     L_segment.start_point))
        return true;
    else if (f4 == 0 && is_intersect(AABB_min_max<Point_2>{segment.start_point, segment.end_point},
                                     L_segment.end_point))
        return true;
    return false;
}

inline bool is_intersect(const Segment<Point_2> &L_segment, const Segment<Point_2> &segment) {
    AABB_min_max<Point_2> L_AABB{L_segment.start_point, L_segment.end_point};
    AABB_min_max<Point_2> R_AABB{segment.start_point, segment.end_point};
    if (!is_intersect(L_AABB, R_AABB)) {
        return false;
    }
    return intersect_pass_AABB(L_segment, segment);
}

template<typename T>
bool is_intersect(const Trapezoid &trapezoid, const Segment<T> &segment) {
    // 分为两个三角形
    if (is_intersect(Triangle<Point_2>{trapezoid.right_upper, trapezoid.left_upper, trapezoid.left_lower}, segment))
        return true;
    if (is_intersect(Triangle<Point_2>{trapezoid.left_lower, trapezoid.right_lower, trapezoid.right_upper}, segment))
        return true;
    return false;
}

template<typename T>
bool is_intersect(const Triangle<T> &triangle, const Segment<T> &segment) {
    const AABB_min_max<Point_2> L_AABB{triangle.a, triangle.b, triangle.c};
    const AABB_min_max<Point_2> R_AABB{segment.start_point, segment.end_point};
    if (!is_intersect(L_AABB, R_AABB)) {
        return false;
    }
    // 先判断 AABB
    // if (intersect(triangle, segment.start_point))  // 概率太小，没有必要，
    //     return true;
    // if (intersect(triangle, segment.end_point))
    //     return true;
    // 线段是否相互
    if (intersect_pass_AABB({triangle.a, triangle.b}, segment))
        return true;
    if (intersect_pass_AABB({triangle.b, triangle.c}, segment))
        return true;
    if (intersect_pass_AABB({triangle.c, triangle.a}, segment))
        return true;
    return false;
}

template<typename T>
float distance(const Segment<T> &segment_L, const Segment<T> &segment_R) {
    if (is_intersect(segment_L, segment_R)) {
        return 0.0f;
    }
    // 比较暴力的一个方法
    auto a = distance(segment_L, segment_R.start_point);
    auto b = distance(segment_L, segment_R.end_point);
    auto c = distance(segment_R, segment_L.start_point);
    auto d = distance(segment_R, segment_L.end_point);
    return std::min<float>(std::min<float>(a, b), std::min<float>(c, d));
}


#endif //HELLO_MAC_OBJECTS_INTERSECT_WITH_SEGMENT_H
