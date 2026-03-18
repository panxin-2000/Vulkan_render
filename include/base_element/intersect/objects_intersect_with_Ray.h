//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_OBJECTS_INTERSECT_WITH_RAY_H
#define HELLO_MAC_OBJECTS_INTERSECT_WITH_RAY_H
#include "base_element/geometry/AABB_bounding_box.h"
#include "base_element/geometry/ray.h"


template<typename T>
inline bool intersect(const AABB_centroid<T> &plane, const Ray<T> &ray) {
    // 一个射线是否与一个包围盒相交
    // 然后是二维和三维的区别
    // 先去看二维的内容
    //
    return false;
}


#endif //HELLO_MAC_OBJECTS_INTERSECT_WITH_RAY_H
