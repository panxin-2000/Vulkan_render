//
// Created by 潘鑫 on 2025/12/7.
//

#ifndef HELLO_MAC_PLANE_H
#define HELLO_MAC_PLANE_H
#include "base_element/point_3.h"


template<typename point_type = Point_3>
struct Plane {
    point_type point;
    point_type normal;
};

#endif //HELLO_MAC_PLANE_H
