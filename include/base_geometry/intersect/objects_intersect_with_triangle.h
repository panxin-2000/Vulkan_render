//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_OBJECTS_INTERSECT_WITH_TRIANGLE_H
#define HELLO_MAC_OBJECTS_INTERSECT_WITH_TRIANGLE_H

#include "objects_intersect_with_point.h"

template<typename T>
float distance(const Triangle<T> &triangle, const T &point) {
    T a2b             = triangle.b - triangle.a;
    T a2c             = triangle.c - triangle.a;
    auto a2p          = point - triangle.a;
    float area        = cross_product(a2b, a2c);        // ABC
    const float gamma = cross_product(a2b, a2p) / area; // ABP
    const float beta  = cross_product(a2p, a2c) / area; // APC
    const float alpha = 1.0f - (gamma + beta);          // PBC

    // P = alpha * A +  beta * B +  gamma * C
    const auto u = alpha;
    const auto v = beta;
    const auto w = gamma;
    if (v <= 0 && w <= 0) {
        // 区域 1: 靠近顶点 A
        return distance(point, triangle.a);
    } else if (u <= 0 && w <= 0) {
        // 区域 2: 靠近顶点 B
        return distance(point, triangle.b);
    } else if (u <= 0 && v <= 0) {
        // 区域 3: 靠近顶点 C
        return distance(point, triangle.c);
    } else if (u <= 0) {
        // 区域 4: 靠近边 BC (进行线段投影)
        return distance(Segment<T>{triangle.b, triangle.c}, point);
    } else if (v <= 0) {
        // 区域 5: 靠近边 AC
        return distance(Segment<T>{triangle.a, triangle.c}, point);
    } else if (w <= 0) {
        // 区域 6: 靠近边 AB
        return distance(Segment<T>{triangle.a, triangle.b}, point);
    } else {
        // 区域 7: 在三角形内部 (u,v,w 均 > 0)
        return 0.0f;
    }
    return 0.0f;
}


#endif //HELLO_MAC_OBJECTS_INTERSECT_WITH_TRIANGLE_H
