//
// Created by 潘鑫 on 2025/6/15.
//

#include "gtest/gtest.h"
#include "vector_signed_area.h"




bool point_2::operator==(const point_2 &R) {
    if (this->x == R.x && this->y == R.y) return true;
    else return false;
}



// 其实感觉不应该用这个的，但是暂时没有办法，就先用这个吧，应该和sort一样，
// 增加一个添加函数的接口的



point_2 point_2::operator-(const point_2 &R) {
    point_2 temp{0, 0};
    temp.x = this->x - R.x;
    temp.y = this->y - R.y;
    return temp;
}

/**
 * 可以用来判断顺时针还是逆时针，第二个相对于第一个逆时针为正，顺时针为负
 * 这里写完还是很简单的
 * @param R
 * @return
 */
float point_2::single_area(const point_2 &R) {
    return this->x * R.y - this->y * R.x;
}

/**
 * 这里的on_segment 函数名并不是很对，只是判断了 测试点 是否在 线段的包围盒内
 * @param segment_start_point
 * @param segment_end_point
 * @param test_point
 * @return
 */
bool on_segment_bounding_box(const point_2 &segment_start_point, point_2 &segment_end_point,
                             point_2 &test_point) {
    if (std::min(segment_start_point.x, segment_end_point.x) <= test_point.x &&
        test_point.x <= std::max(segment_start_point.x, segment_end_point.x) &&
        std::min(segment_start_point.y, segment_end_point.y) <= test_point.y &&
        test_point.y <= std::max(segment_start_point.y, segment_end_point.y))
        return true;
    return false;
}

bool segment_position::get_intersection_point(struct segment_position &R_segment_position, point_2 *result) {
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
    point_2 ab = this->end_point - this->start_point;
    float a_0 = ab.y / ab.x;
    float b_0 = this->start_point.y - a_0 * this->start_point.x;
    point_2 cd = R_segment_position.end_point - R_segment_position.start_point;
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

bool segment_position::intersection(struct segment_position &R_segment_position) {
    point_2 ab = this->end_point - this->start_point;
    point_2 ac = R_segment_position.start_point - this->start_point;
    point_2 ad = R_segment_position.end_point - this->start_point;

    point_2 cd = R_segment_position.end_point - R_segment_position.start_point;
    point_2 ca = this->start_point - R_segment_position.start_point;
    point_2 cb = this->end_point - R_segment_position.start_point;

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
    if (f1 == 0 && on_segment_bounding_box(this->start_point, this->end_point, R_segment_position.start_point))
        return true;
    if (f2 == 0 && on_segment_bounding_box(this->start_point, this->end_point, R_segment_position.end_point))
        return true;
    if (f3 == 0 && on_segment_bounding_box(R_segment_position.start_point, R_segment_position.end_point,
                                           this->start_point))
        return true;
    if (f4 == 0 && on_segment_bounding_box(R_segment_position.start_point, R_segment_position.end_point,
                                           this->end_point))
        return true;
    return false;
}
