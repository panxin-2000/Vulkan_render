//
// Created by 潘鑫 on 2026/7/29.
//

#ifndef HELLO_MAC_FRUSTUM_H
#define HELLO_MAC_FRUSTUM_H


#include <Eigen/Eigen>

struct FrustumPlanes {
    std::array<Eigen::Vector4f, 6> planes = {};
};

struct Frustum_cull_flag {
};


#endif //HELLO_MAC_FRUSTUM_H
