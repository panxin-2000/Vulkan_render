//
// Created by 潘鑫 on 2025/12/7.
//

#ifndef HELLO_MAC_PLANE_H
#define HELLO_MAC_PLANE_H
#include "base_geometry/base.h"


template<typename point_type = Point_3>
struct Plane {
public:
    point_type point;
    point_type normal;


    float distance(point_type q) {
        return dot(normal, (q - point)) / dot(normal, normal);
    }
};

#endif //HELLO_MAC_PLANE_H
