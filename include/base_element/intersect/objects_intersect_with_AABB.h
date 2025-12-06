//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_OBJECTS_INTERSECT_WITH_AABB_H
#define HELLO_MAC_OBJECTS_INTERSECT_WITH_AABB_H
#include "base_element/geometry/AABB_bounding_box.h"

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
    if (abs((L_box.centroid_point - R_box.centroid_point)) <=
        abs((L_box.direction_interval + R_box.direction_interval)))
        return true;
    return false;
}


#endif //HELLO_MAC_OBJECTS_INTERSECT_WITH_AABB_H
