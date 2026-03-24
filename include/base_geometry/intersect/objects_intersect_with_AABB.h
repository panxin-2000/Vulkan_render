//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_OBJECTS_INTERSECT_WITH_AABB_H
#define HELLO_MAC_OBJECTS_INTERSECT_WITH_AABB_H
#include "base_geometry/base.h"

template<typename T>
bool intersect(const AABB_min_max<T> &L_box, const AABB_min_max<T> &R_box) {
    // 这里尽量还是转换之后再去判断会更好一点
    if (intersect(AABB_centroid<T>(L_box), AABB_centroid<T>(R_box))) {
        return true;
    }
    return false;
}


/**
 *  应该是需要分开坐标轴去判断
 * @tparam T
 * @param L_box
 * @param R_box
 * @return
 */
template<typename T>
inline bool intersect(const AABB_centroid<T> &L_box, const AABB_centroid<T> &R_box) {
    if (abs((L_box.centroid_point_ - R_box.centroid_point_)) <=
        abs((L_box.direction_interval_ + R_box.direction_interval_)))
        return true;
    return false;
}


template<typename T>
float distance(const AABB_min_max<T> &box, const T &test_point) {
    auto projection = clamp(test_point, box.min_point_, box.max_point_);
    // auto error      = clamp({}, box.max_point_ - test_point, test_point - box.min_point_);
    return dot({test_point - projection}, {test_point - projection});
}


#endif //HELLO_MAC_OBJECTS_INTERSECT_WITH_AABB_H
