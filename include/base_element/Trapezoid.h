//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_TRAPEZOID_H
#define HELLO_MAC_TRAPEZOID_H
#include "base_element/point_2.h"


struct Trapezoid {
    Point_2 left_upper;
    Point_2 right_upper;
    Point_2 left_lower;
    Point_2 right_lower;
};
#endif //HELLO_MAC_TRAPEZOID_H
