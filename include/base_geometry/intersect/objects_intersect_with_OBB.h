//
// Created by 潘鑫 on 2026/3/24.
//

#ifndef HELLO_MAC_OBJECTS_INTERSECT_WITH_OBB_H
#define HELLO_MAC_OBJECTS_INTERSECT_WITH_OBB_H
#include "base_geometry/base.h"

template<typename T>
inline bool is_intersect(const OBB_2D<T> &obb, const T &test_point) {
    const auto P_to_O = test_point - obb.centroid_;
    const auto u      = (P_to_O.dot(obb.direction_1));
    const auto v      = (P_to_O.dot(obb.direction_2));
    if constexpr (std::is_same_v<std::decay_t<T>, Point_2>) {
        if ((std::abs(u) - obb.interval_.x > 0) || (std::abs(v) - obb.interval_.y > 0)) {
            return false;
        }
        return true;
    } else if constexpr (std::is_same_v<std::decay_t<T>, Point_3>) {
        const auto direction_3 = cross_product(obb.direction_1, obb.direction_2);
        const auto w           = (P_to_O.dot(direction_3));
        if ((std::abs(u) - obb.interval_.x > 0) ||
            (std::abs(v) - obb.interval_.y > 0) ||
            (std::abs(w) - obb.interval_.z > 0)) {
            return false;
        }
        return true;
    }
}

template<typename T>
inline float distance(const OBB_2D<T> &obb, const T &test_point) {
    static_assert(std::is_same_v<std::decay_t<T>, Point_2> || std::is_same_v<std::decay_t<T>, Point_3>);
    const auto P_to_O = test_point - obb.centroid_;
    const auto u      = (P_to_O.dot(obb.direction_1));
    const auto v      = (P_to_O.dot(obb.direction_2));
    if constexpr (std::is_same_v<std::decay_t<T>, Point_2>) {
        const auto value_u = std::clamp(std::abs(u) - obb.interval_.x, 0.0f,INFINITY);
        const auto value_v = std::clamp(std::abs(v) - obb.interval_.y, 0.0f,INFINITY);
        return value_u * value_u + value_v * value_v;
    } else if constexpr (std::is_same_v<std::decay_t<T>, Point_3>) {
        const auto direction_3 = cross_product(obb.direction_1, obb.direction_2);
        const auto w           = (P_to_O.dot(direction_3));
        const auto value_u     = std::clamp(std::abs(u) - obb.interval_.x, 0.0f,INFINITY);
        const auto value_v     = std::clamp(std::abs(v) - obb.interval_.y, 0.0f,INFINITY);
        const auto value_w     = std::clamp(std::abs(w) - obb.interval_.z, 0.0f,INFINITY);
        return value_u * value_u + value_v * value_v + value_w * value_w;
    }
    return NAN;
}


#endif //HELLO_MAC_OBJECTS_INTERSECT_WITH_OBB_H
