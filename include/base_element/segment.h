//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_SEGMENT_H
#define HELLO_MAC_SEGMENT_H
#include "base_element/base.h"
#include "iostream"

template<typename point_type = Point_2>
struct Segment {
    point_type start_point;
    point_type end_point;


    static point_type get_intersection_point(point_type &start_point, point_type &end_point, float x) {
        point_type ab = start_point - end_point;
        point_type result;
        float a_0 = ab.y / ab.x; // a_0 是 start_point 到 end_point 之间的斜率
        result.x = x;
        result.y = start_point.y + a_0 * (x - start_point.x);
        return result;
    }

    bool get_intersection_point(struct Segment &R_segment_position, Point_2 *result) {
        // 已知两条线段相交怎么求交点？
        // y_0 = a_0 * x + b_0
        // y_1 = a_1 * x + b_1
        // a_0 * x + b_0 = a_1 * x + b_1
        //  b_0 - b_1  = (a_1 - a_0) * x
        //  b_0 - b_1
        // -----------  =  X
        //  a_1 - a_0
        //  y = a_0 * x + b_0
        // a_1 - a_0 == 0 时 为平行线
        Point_2 ab = this->end_point - this->start_point;
        float a_0 = ab.y / ab.x;
        float b_0 = this->start_point.y - a_0 * this->start_point.x;
        Point_2 cd = R_segment_position.end_point - R_segment_position.start_point;
        float a_1 = cd.y / cd.x;
        float b_1 = R_segment_position.start_point.y - a_1 * R_segment_position.start_point.x;
        if (std::abs(a_1 - a_0) < 0.000001) {
            //错误的
            return false;
        } else {
            result->x = (b_0 - b_1) / (a_1 - a_0);
            result->y = a_0 * result->x + b_0;
            return true;
        }
    }
};

#endif //HELLO_MAC_SEGMENT_H
