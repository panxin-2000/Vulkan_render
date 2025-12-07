//
// Created by 潘鑫 on 2025/10/21.
//

#ifndef SEGMENT_END_PONIT_AND_GRADIENT_H
#define SEGMENT_END_PONIT_AND_GRADIENT_H
#include "half_edge.h"

struct ray_2d {
    float x, y;
    float gradient;
    float compare_x_position;
    int incident_half_edge;
    // 只给了点与射线，没有给需要比较多位置
    bool operator<(const ray_2d &right) const {
        float current_segment_y = y + gradient * (right.compare_x_position - x);
        float right_segment_y = right.y + right.gradient * (right.compare_x_position - right.x);
        if (current_segment_y < right_segment_y) {
            return true;
        }
        return false;
    }

    bool operator==(const ray_2d &right) const {
        if (x == right.x && y == right.y && gradient == right.gradient) {
            return true;
        }
        return false;
    }

    static ray_2d &
    get_ray_2d(half_edge_struct<vertex_xy> &hf, int incident_half_edge);
};


#endif //SEGMENT_END_PONIT_AND_GRADIENT_H
