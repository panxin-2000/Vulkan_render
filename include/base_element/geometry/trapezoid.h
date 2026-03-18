//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_TRAPEZOID_H
#define HELLO_MAC_TRAPEZOID_H
#include "point_2.h"


struct Trapezoid {
    Point_2 left_upper;
    Point_2 right_upper;
    Point_2 left_lower;
    Point_2 right_lower;

public:
    Point_2 get_centroid() const {
        return {(left_upper + right_upper + left_lower + right_lower) / 4};
    }
};
#endif //HELLO_MAC_TRAPEZOID_H
