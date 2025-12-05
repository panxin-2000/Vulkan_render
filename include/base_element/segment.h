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
