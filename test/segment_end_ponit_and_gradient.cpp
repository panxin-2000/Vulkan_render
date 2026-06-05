//
// Created by 潘鑫 on 2025/10/21.
//
#include "segment_end_ponit_and_gradient.h"


ray_2d &ray_2d::get_ray_2d(
    Half_edges<vertex_xy> &hf, int incident_half_edge) {
    ray_2d *temp = new ray_2d;
    Segment<Point_2> current_segment = hf.get_segment(incident_half_edge);
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
