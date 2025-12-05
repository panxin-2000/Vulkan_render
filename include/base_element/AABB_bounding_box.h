//
// Created by 潘鑫 on 2025/11/1.
//

#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H
#include "point_2.h"
#include "segment.h"

template<typename T>
class AABB {
public:
    T min_point; // 最小点，并不一定是真实存在的点，可能是由两个点拼出来的一个点
    T max_point; // 最大点也是一样的


    AABB() {
        min_point = T::int_max_limit(min_point);
        max_point = T::int_min_limit(max_point);
    }

    AABB(std::initializer_list<T> points) {
        min_point = T::int_max_limit(min_point);
        max_point = T::int_min_limit(max_point);
        for (auto vertex_point: points) {
            min_point = T::min_two_point(min_point, vertex_point);
            max_point = T::max_two_point(max_point, vertex_point);
        }
    }

    AABB(T l_points, T r_points) {
        min_point = T::int_max_limit(min_point);
        max_point = T::int_min_limit(max_point);
        min_point = T::min_two_point(min_point, l_points);
        max_point = T::max_two_point(max_point, l_points);
        min_point = T::min_two_point(min_point, r_points);
        max_point = T::max_two_point(max_point, r_points);
    }


    static AABB calculate_bound_box(std::vector<T> &points) {
        AABB box;
        for (auto vertex_point: points) {
            box.min_point = T::min_two_point(box.min_point, vertex_point);
            box.max_point = T::max_two_point(box.max_point, vertex_point);
        }
        return box;
    }

    /**
     * 在包围盒的内部和边缘的线上都 返回 true
     * @param box
     * @param test_point 需要测试 是否 在包围盒内的点
     * @return
     */
};

template<typename T>
bool intersect(const AABB<T> &box, const T &test_point) {
    if (box.min_point <= test_point && test_point <= box.max_point)
        return true;
    return false;
}

inline bool intersect(segment_position<Point_2> &L_segment, segment_position<Point_2> &R_segment) {
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
    if (f1 == 0 && intersect({L_segment.start_point, L_segment.end_point},
                             R_segment.start_point))
        return true;
    if (f2 == 0 && intersect({L_segment.start_point, L_segment.end_point},
                             R_segment.end_point))
        return true;
    if (f3 == 0 && intersect({R_segment.start_point, R_segment.end_point},
                             L_segment.start_point))
        return true;
    if (f4 == 0 && intersect({R_segment.start_point, R_segment.end_point},
                             L_segment.end_point))
        return true;
    return false;
}

#endif //BOUNDING_BOX_H
