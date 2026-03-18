//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_OBJECTS_INTERSECT_WITH_RAY_H
#define HELLO_MAC_OBJECTS_INTERSECT_WITH_RAY_H


#include "base_element/base.h"



// template<typename T>
// inline bool intersect(const AABB_centroid<T> &plane, const Ray<T> &ray) {
//     // 一个射线是否与一个包围盒相交
//     // 然后是二维和三维的区别
//     // 先去看二维的内容
//     //
//     return false;
// }

inline bool intersect(const AABB_min_max<Point_2> &L_box, const Ray<Point_2> &segment) {
}

inline bool intersect(const AABB_min_max<Point_3> &L_box, const Ray<Point_3> &segment) {
}


template<typename T>
inline bool intersect(const AABB_centroid<T> &L_box, const Ray<T> &segment) {
    return intersect(AABB_min_max<T>(L_box), segment);
}


#endif //HELLO_MAC_OBJECTS_INTERSECT_WITH_RAY_H
