//
// Created by 潘鑫 on 2025/10/21.
//
#include "segment_end_ponit_and_gradient.h"


segment_start_point_and_gradient &segment_start_point_and_gradient::get_segment_start_point_and_gradient(
    half_edge_struct<vertex_xy> &hf, int incident_half_edge) {
    segment_start_point_and_gradient *temp = new segment_start_point_and_gradient;
    segment_position current_segment = hf.get_segment(incident_half_edge);
    temp->compare_x_position = current_segment.start_point.x;
    if (current_segment.end_point.x < current_segment.start_point.x) {
        std::swap(current_segment.start_point, current_segment.end_point);
    }
    temp->x = current_segment.start_point.x;
    temp->y = current_segment.start_point.y;
    temp->incident_half_edge = incident_half_edge;
    temp->gradient = (current_segment.end_point.y - current_segment.start_point.y) /
                     (current_segment.end_point.x - current_segment.start_point.x);
    return *temp;
}
