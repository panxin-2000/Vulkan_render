//
// Created by 潘鑫 on 2026/3/24.
//

#ifndef HELLO_MAC_OBJECTS_INTERSECT_WITH_OBB_H
#define HELLO_MAC_OBJECTS_INTERSECT_WITH_OBB_H
#include "base_geometry/base.h"


inline bool intersect(const OBB_2D &obb, const Point_2 &test_point) {
    const auto P_to_O = test_point - obb.centroid_;
    const auto u      = dot(P_to_O, obb.direction_1);
    const auto v      = dot(P_to_O, obb.direction_2);
    if ((std::abs(u) - obb.interval_.x > 0) || (std::abs(v) - obb.interval_.y > 0)) {
        return false;
    }
    return true;
}

inline float distance(const OBB_2D &obb, const Point_2 &test_point) {
    const auto P_to_O  = test_point - obb.centroid_;
    const auto u       = dot(P_to_O, obb.direction_1);
    const auto v       = dot(P_to_O, obb.direction_2);
    const auto value_u = std::clamp(std::abs(u) - obb.interval_.x, 0.0f,INFINITY);
    const auto value_v = std::clamp(std::abs(v) - obb.interval_.y, 0.0f,INFINITY);
    return value_u * value_u + value_v * value_v;
}


#endif //HELLO_MAC_OBJECTS_INTERSECT_WITH_OBB_H
