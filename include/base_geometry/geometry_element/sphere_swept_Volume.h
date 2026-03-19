//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_SPHERE_SWEPT_VOLUME_H
#define HELLO_MAC_SPHERE_SWEPT_VOLUME_H
#include "AABB_bounding_box.h"
#include "Oriented_Bounding_Boxes.h"
#include "segment.h"

template<typename T>
class Sphere_sweep_line {
public:
    Segment<T> segment;
    T center;     // 球心
    float radius; // 半径
};

template<typename T>
class Sphere_sweep_AABB {
public:
    AABB_centroid<T> box;
    T center; // 球心
    T radius; // 半径
};

template<typename T>
class Sphere_sweep_OBB {
public:
    OBB<T> obb_box;
    T center; // 球心
    T radius; // 半径
};

#endif //HELLO_MAC_SPHERE_SWEPT_VOLUME_H
