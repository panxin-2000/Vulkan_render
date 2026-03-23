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

    Plane(const point_type &point, const point_type &normal) : point(point), normal(normal) {
        assert(dot(normal,normal ) == 1);
    }
};

#endif //HELLO_MAC_PLANE_H
