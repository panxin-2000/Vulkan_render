//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_SEGMENT_H
#define HELLO_MAC_SEGMENT_H
#include "base_element/base.h"
#include "iostream"

struct segment_position {
    Point_2 start_point;
    Point_2 end_point;


    bool intersection(struct segment_position &R_segment_position) {
        Point_2 ab = this->end_point - this->start_point;
        Point_2 ac = R_segment_position.start_point - this->start_point;
        Point_2 ad = R_segment_position.end_point - this->start_point;

        Point_2 cd = R_segment_position.end_point - R_segment_position.start_point;
        Point_2 ca = this->start_point - R_segment_position.start_point;
        Point_2 cb = this->end_point - R_segment_position.start_point;

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
        if (f1 == 0 && on_segment_bounding_box(this->start_point, this->end_point,
                                               R_segment_position.start_point))
            return true;
        if (f2 == 0 && on_segment_bounding_box(this->start_point, this->end_point,
                                               R_segment_position.end_point))
            return true;
        if (f3 == 0 && on_segment_bounding_box(R_segment_position.start_point, R_segment_position.end_point,
                                               this->start_point))
            return true;
        if (f4 == 0 && on_segment_bounding_box(R_segment_position.start_point, R_segment_position.end_point,
                                               this->end_point))
            return true;
        return false;
    }

    static bool on_segment_bounding_box(const Point_2 &segment_start_point, Point_2 &segment_end_point,
                                        Point_2 &test_point) {
        // std::cout << "segment_start_point " << segment_start_point << std::endl;
        // std::cout << "segment_end_point " << segment_end_point << std::endl;
        // std::cout << "test_point " << test_point << std::endl;
        if (std::min(segment_start_point.x, segment_end_point.x) <= test_point.x &&
            test_point.x <= std::max(segment_start_point.x, segment_end_point.x) &&
            std::min(segment_start_point.y, segment_end_point.y) <= test_point.y &&
            test_point.y <= std::max(segment_start_point.y, segment_end_point.y))
            return true;
        return false;
    }

    bool get_intersection_point(struct segment_position &R_segment_position, Point_2 *result);


    static Point_2 get_intersection_point(Point_2 &start_point, Point_2 &end_point, float x) {
        Point_2 ab = start_point - end_point;
        Point_2 result;
        float a_0 = ab.y / ab.x; // a_0 是 start_point 到 end_point 之间的斜率
        result.x = x;
        result.y = start_point.y + a_0 * (x - start_point.x);
        return result;
    }
};

#endif //HELLO_MAC_SEGMENT_H
