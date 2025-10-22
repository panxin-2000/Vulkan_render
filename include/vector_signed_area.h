//
// Created by 潘鑫 on 2025/10/1.
//

#ifndef VECTOR_SIGNED_AREA_H
#define VECTOR_SIGNED_AREA_H

#include <vector>

class point_2 {
public:
    float x;
    float y;

    point_2() {
    }

    point_2(float x1, float y1);

    point_2 operator+(const point_2 &R);

    bool operator==(const point_2 &R);


    point_2 operator-(const point_2 &R);

    float single_area(const point_2 &R);
};

bool operator==(const point_2 &L, const point_2 &R);

bool operator<(const point_2 &L, const point_2 &R);


typedef point_2 triangle_position;

struct triangle {
    triangle_position a;
    triangle_position b;
    triangle_position c;
};

bool operator==(const triangle &L, const triangle &R);


bool on_segment_bounding_box(const point_2 &segment_start_point, point_2 &segment_end_point,
                             point_2 &test_point);

struct segment_position {
    point_2 start_point;
    point_2 end_point;

    bool intersection(struct segment_position &R_segment_position);

    bool get_intersection_point(struct segment_position &R_segment_position, point_2 *result);
};

bool convex_hull_in_order_of_angles(std::vector<point_2> &new_segments);

std::vector<point_2> &calculate_convex_hull(std::vector<point_2> &segments);
#endif //VECTOR_SIGNED_AREA_H
