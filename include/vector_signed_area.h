//
// Created by 潘鑫 on 2025/10/1.
//

#ifndef VECTOR_SIGNED_AREA_H
#define VECTOR_SIGNED_AREA_H

class segment_vector {
public:
    float x;
    float y;

    segment_vector(float x1, float y1);

    segment_vector operator+(const segment_vector &R);


    segment_vector operator-(const segment_vector &R);

    float single_area(const segment_vector &R);
};

bool operator==(const segment_vector &L, const segment_vector &R);


typedef segment_vector triangle_position;

struct triangle {
    triangle_position a;
    triangle_position b;
    triangle_position c;
};

bool on_segment(const segment_vector &a, segment_vector &b, segment_vector &c);

struct segment_position {
    segment_vector start_point;
    segment_vector end_point;

    bool intersection(struct segment_position &R_segment_position);
};

bool convex_hull(std::vector<segment_vector> &new_segments);

#endif //VECTOR_SIGNED_AREA_H
