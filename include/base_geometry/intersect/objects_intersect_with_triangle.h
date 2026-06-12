//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_OBJECTS_INTERSECT_WITH_TRIANGLE_H
#define HELLO_MAC_OBJECTS_INTERSECT_WITH_TRIANGLE_H

#include "objects_intersect_with_point.h"
#include "ccd/ccd.h"

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

inline void my_support(const void *obj, const ccd_vec3_t *dir, ccd_vec3_t *vec) {
    // 1. 强制转回你的连续内存容器
    const auto &mesh = *static_cast<const std::vector<Point_3> *>(obj);

    float max_dot = -FLT_MAX;
    int best_idx  = 0;

    // 2. 直接在连续内存上进行点积（性能极高）
    for (size_t i = 0; i < mesh.size(); ++i) {
        float dot = mesh[i].x * dir->v[0] + mesh[i].y * dir->v[1] + mesh[i].z * dir->v[2];
        if (dot > max_dot) {
            max_dot  = dot;
            best_idx = i;
        }
    }

    // 3. 将结果写回给 libccd
    vec->v[0] = mesh[best_idx].x;
    vec->v[1] = mesh[best_idx].y;
    vec->v[2] = mesh[best_idx].z;
}


template<typename T>
bool is_intersect(const Triangle<T> &L, const Triangle<T> &R) {
    ccd_t ccd;
    CCD_INIT(&ccd);

    // 设置回调函数
    ccd.support1       = my_support;
    ccd.support2       = my_support;
    ccd.max_iterations = 100;    // 迭代次数限制
    ccd.epa_tolerance  = 0.0001; // maximal tolerance fro EPA part

    std::vector<Point_3> meshA = {L.a, L.b, L.c};
    std::vector<Point_3> meshB = {R.a, R.b, R.c};

    // 直接传入 vector 的地址即可
    int intersect = ccdGJKIntersect(&meshA, &meshB, &ccd);

    if (intersect) {
        // 发生了碰撞！
        return true;
    }
    return false;
}


#endif //HELLO_MAC_OBJECTS_INTERSECT_WITH_TRIANGLE_H
