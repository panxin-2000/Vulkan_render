//
// Created by 潘鑫 on 2025/6/15.
//

#include "gtest/gtest.h"
#include "vector_signed_area.h"

segment_vector::segment_vector(float x1, float y1) {
    x = x1;
    y = y1;
}

segment_vector segment_vector::operator+(const segment_vector &R) {
    segment_vector temp{0, 0};
    temp.x = this->x + R.x;
    temp.y = this->y + R.y;
    return temp;
}

bool operator==(const segment_vector &L, const segment_vector &R) {
    if (abs(L.y - R.y) < 0.001 && abs(L.x - R.x) < 0.001) {
        return true;
    }
    return false;
}

bool operator==(const triangle &L, const triangle &R) {
    if (L.a == R.a && L.b == R.b && L.c == R.c) {
        return true;
    }
    return false;
}

segment_vector segment_vector::operator-(const segment_vector &R) {
    segment_vector temp{0, 0};
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
float segment_vector::single_area(const segment_vector &R) {
    return this->x * R.y - this->y * R.x;
}


bool on_segment(const segment_vector &a, segment_vector &b, segment_vector &c) {
    if (std::min(a.x, b.x) <= c.x &&
        c.x <= std::max(a.x, b.x) &&
        std::min(a.y, b.y) <= c.y &&
        c.y <= std::max(a.y, b.y))
        return true;
    return false;
}


bool segment_position::intersection(struct segment_position &R_segment_position) {
    segment_vector ab = this->end_point - this->start_point;
    segment_vector ac = R_segment_position.start_point - this->start_point;
    segment_vector ad = R_segment_position.end_point - this->start_point;

    segment_vector cd = R_segment_position.end_point - R_segment_position.start_point;
    segment_vector ca = this->start_point - R_segment_position.start_point;
    segment_vector cb = this->end_point - R_segment_position.start_point;

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
    if (f1 == 0 && on_segment(this->start_point, this->end_point, R_segment_position.start_point))
        return true;
    if (f2 == 0 && on_segment(this->start_point, this->end_point, R_segment_position.end_point))
        return true;
    if (f3 == 0 && on_segment(R_segment_position.start_point, R_segment_position.end_point, this->start_point))
        return true;
    if (f3 == 0 && on_segment(R_segment_position.start_point, R_segment_position.end_point, this->end_point))
        return true;
    return false;
}
