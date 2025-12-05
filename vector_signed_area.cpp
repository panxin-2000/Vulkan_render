//
// Created by 潘鑫 on 2025/6/15.
//

#include "gtest/gtest.h"
#include "vector_signed_area.h"
#include "base_element/segment.h"


/**
 * 这里的on_segment 函数名并不是很对，只是判断了 测试点 是否在 线段的包围盒内
 * @param segment_start_point
 * @param segment_end_point
 * @param test_point
 * @return
 */


bool segment_position::get_intersection_point(struct segment_position &R_segment_position, Point_2 *result) {
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
