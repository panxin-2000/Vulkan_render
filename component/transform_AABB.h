//
// Created by 潘鑫 on 2026/8/14.
//

#ifndef HELLO_MAC_TRANSFORM_AABB_H
#define HELLO_MAC_TRANSFORM_AABB_H


#include "transform_component.h"
#include "AABB_box.h"

inline Render_AABB transform_AABB(const Render_AABB &bound_box, const Transform_Matrix &matrix) {
    const Eigen::Vector4f new_centroid  = matrix.get() * bound_box.centroid_points;
    const Eigen::Matrix3f R             = matrix.get().block<3, 3>(0, 0);
    const Eigen::Vector3f new_direction = R.cwiseAbs() * bound_box.direction_intervals.head<3>();
    return {
        {new_centroid.x(), new_centroid.y(), new_centroid.z(), 1.0f},
        {new_direction.x(), new_direction.y(), new_direction.z(), 0.0f}
    };
}




#endif //HELLO_MAC_TRANSFORM_AABB_H
