//
// Created by 潘鑫 on 2026/8/13.
//

#ifndef HELLO_MAC_AABB_BOX_H
#define HELLO_MAC_AABB_BOX_H

#include <Eigen/Eigen>

struct alignas(16) Render_AABB {
    Eigen::Vector4f centroid_points;
    Eigen::Vector4f direction_intervals;
};

#endif //HELLO_MAC_AABB_BOX_H
