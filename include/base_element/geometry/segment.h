//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_SEGMENT_H
#define HELLO_MAC_SEGMENT_H
#include "base_element/point_2.h"
#include "iostream"

template<typename point_type = Point_2>
struct Segment {
    point_type start_point;
    point_type end_point;

    friend std::ostream &operator<<(std::ostream &output,
                                    const Segment &P) {
        output << "start " << P.start_point << " end " << P.end_point;
        return output;
    }


    static point_type get_segment_point_on_x(point_type &start_point, point_type &end_point, float x) {
        point_type ab = start_point - end_point;
        point_type result;
        float a_0 = ab.y / ab.x; // a_0 是 start_point 到 end_point 之间的斜率
        result.x = x;
        result.y = start_point.y + a_0 * (x - start_point.x);
        return result;
    }

    /**
     * 这个函数有一个问题，那就是我应该是已知相交
     * @param R_segment_position
     * @param result
     * @return
     */
    Point_2 get_intersect_result(Segment &R_segment_position) {
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
        const Point_2 ab = this->end_point - this->start_point;
        float a_0 = ab.y / ab.x;
        float b_0 = this->start_point.y - a_0 * this->start_point.x;
        const Point_2 cd = R_segment_position.end_point - R_segment_position.start_point;
        float a_1 = cd.y / cd.x;
        const float b_1 = R_segment_position.start_point.y - a_1 * R_segment_position.start_point.x;
        const auto x_temp = (b_0 - b_1) / (a_1 - a_0);
        const Point_2 result{x_temp, a_0 * x_temp + b_0};
        return result;
    }
};

#endif //HELLO_MAC_SEGMENT_H
