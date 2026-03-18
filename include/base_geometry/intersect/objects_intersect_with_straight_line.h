//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_OBJECTS_INTERSECT_WITH_STRAIGHT_LINE_H
#define HELLO_MAC_OBJECTS_INTERSECT_WITH_STRAIGHT_LINE_H
#include "base_geometry/base.h"


template<typename T>
inline bool intersect(const AABB_centroid<T> &L_box, const Straight_line<T> &line) {
    return intersect(AABB_min_max<Point_2>(L_box), line);
}

inline bool intersect(const AABB_min_max<Point_2> &L_box, const Straight_line<Point_2> &line) {
    Point_2 box_min_x_min_y = {L_box.min_point.x, L_box.min_point.y};
    Point_2 box_min_x_max_y = {L_box.min_point.x, L_box.max_point.y};
    Point_2 box_mam_x_min_y = {L_box.max_point.x, L_box.min_point.y};
    Point_2 box_max_x_max_y = {L_box.max_point.x, L_box.max_point.y};
    auto bool_1             = Point_2::is_anticlockwise(line.point, line.point + line.direction,
                                            box_min_x_min_y);
    auto bool_2 = Point_2::is_anticlockwise(line.point, line.point + line.direction,
                                            box_min_x_max_y);
    auto bool_3 = Point_2::is_anticlockwise(line.point, line.point + line.direction,
                                            box_mam_x_min_y);
    auto bool_4 = Point_2::is_anticlockwise(line.point, line.point + line.direction,
                                            box_max_x_max_y);
    if (((bool_1 | bool_2 | bool_3 | bool_4) == Point_2::anticlockwise::counterclockwise) ||
        ((bool_1 | bool_2 | bool_3 | bool_4) == Point_2::anticlockwise::clockwise)) {
        // 只有单一的一种必然是不相交的
        return false;
    }
    return true;
}

template<typename T>
bool intersect(const Sphere<T> &sphere, const Straight_line<T> &line) {
    auto center_to_ray_start = line.point - sphere.center;
    auto c                   = (dot(center_to_ray_start, center_to_ray_start) - sphere.radius * sphere.radius);
    if (c < 0) {
        // 如果是直线的话，这个分支概率很小，几乎接近零
        return true;
    }
    auto direction  = line.direction;
    auto b_half     = dot(center_to_ray_start, direction);
    auto a          = dot(direction, direction);
    auto delta_half = b_half * b_half - dot(direction, direction) * c;
    if (delta_half < 0) {
        return false;
    }
    return true;
}


#endif //HELLO_MAC_OBJECTS_INTERSECT_WITH_STRAIGHT_LINE_H
