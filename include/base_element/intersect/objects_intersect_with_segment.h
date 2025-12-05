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

template<typename T>
bool intersect(const AABB_min_max<T> &L_box, const Segment<T> &R_segment) {
    /**
     *     找到四个 # 号的点，判断是否点是否在包围盒上并且在线段内，在则相交
     *     *
     *        *
     *           *
     *              #
     *                 *
     *              * * * # * * * * * * * * *
     *              *        *              *
     *              *           *           *
     *              *              *        *
     *              *                 *     *
     *              *                    *  *
     *              *                       #
     *              *                       *  *
     *              * * * * * * * * * * * * *     #
     *                                               *
     *                                                  *
     */


    // 存在相交
    return true;

    return false;

    // 求最近点时，有一个稍微简单一点点办法，比较偏向于直线了
    // 找到最近点两个 # 点
    // 利用相似三角形可以比较快的得出结果
}

template<typename T>
bool intersect_with_closest_result(const AABB_min_max<T> &L_box, const Segment<T> &R_segment, const T *result) {

    // 求最近点时，有一个稍微简单一点点办法，比较偏向于直线了
    // 找到最近点两个 # 点
    // 利用相似三角形可以比较快的得出结果
}
#endif //HELLO_MAC_OBJECTS_INTERSECT_WITH_SEGMENT_H
