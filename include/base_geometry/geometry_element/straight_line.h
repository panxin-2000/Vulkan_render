//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_STRAIGHT_LINE_H
#define HELLO_MAC_STRAIGHT_LINE_H
#include "point_2.h"

template<typename point_type = Point_2>
struct Straight_line {
    point_type point;
    point_type direction;
};

#endif //HELLO_MAC_STRAIGHT_LINE_H
