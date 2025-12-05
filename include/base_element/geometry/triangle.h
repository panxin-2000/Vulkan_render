//
// Created by 潘鑫 on 2025/10/1.
//

#ifndef HELLO_MAC_TRIANGLE_H
#define HELLO_MAC_TRIANGLE_H

#include <vector>
#include <ostream>
#include "../point_2.h"
#include "../../point_in_on_out_triangle.h"
#include "AABB_bounding_box.h"


template<typename T>
struct Triangle {
    T a;
    T b;
    T c;

    Triangle(T a_1, T b_1, T c_1) {
        a = a_1;
        b = b_1;
        c = c_1;
    }

    bool operator==(const Triangle &R) {
        if (!(this->a + this->b + this->c == R.a + R.b + R.c)) {
            return false;
        }
        if ((this->a == R.a && this->b == R.b && this->c == R.c) ||
            (this->a == R.b && this->b == R.c && this->c == R.a) ||
            (this->a == R.c && this->b == R.a && this->c == R.b))
            return true;
        else return false;
    }

    bool operator<(const Triangle &R) const {
        auto left = this->a + this->b + this->c;
        auto right = R.a + R.b + R.c;
        if (left < right) {
            return true;
        }
        return false;
    }

    friend std::ostream &operator<<(std::ostream &output,
                                    const Triangle &D) {
        auto right = D.a + D.b + D.c;

        output << " barycenter x : " << right.x << " barycenter y : " << right.y;
        output << " a.x :  " << D.a.x <<
                " a.y :  " << D.a.y <<
                " b.x :  " << D.b.x <<
                " b.y :  " << D.b.y <<
                " c.x :  " << D.c.x <<
                " c.y :  " << D.c.y << std::endl;
        return output;
    }

    friend bool operator==(const Triangle &L, const Triangle &R) {
        if (L == R) {
            return true;
        }
        return false;
    }

    // 这个算法不太好，先调用intersect，为true之后再调用这个，用于得到三个分量
    // 之后再想想办法，看看能否再优化了
    void point_position_of_triangle(T point, float &alpha, float &beta, float &gamma) {
        T a2b = b - a;
        T a2c = c - a;
        auto a2p = point - a;
        float area = a2b.single_area(a2c);
        alpha = a2p.single_area(a2c) / area;
        beta = a2b.single_area(a2p) / area;
        gamma = 1.0f - (alpha + beta);
    }
};


#endif //HELLO_MAC_TRIANGLE_H
