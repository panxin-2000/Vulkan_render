//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_OBJECTS_INTERSECT_WITH_SPHERE_H
#define HELLO_MAC_OBJECTS_INTERSECT_WITH_SPHERE_H
#include "base_element/geometry/segment.h"
#include "base_element/geometry/Sphere_bounding_volume.h"
#include "base_element/geometry/triangle.h"


template<typename T>
bool intersect(const Sphere<T> &sphere, const Triangle<T> &triangle) {
    if (intersect(sphere, Segment<T>{triangle.a, triangle.b})) {
        return true;
    }
    if (intersect(sphere, Segment<T>{triangle.b, triangle.c})) {
        return true;
    }
    if (intersect(sphere, Segment<T>{triangle.c, triangle.a})) {
        return true;
    }
    return false;
}
#endif //HELLO_MAC_OBJECTS_INTERSECT_WITH_SPHERE_H
