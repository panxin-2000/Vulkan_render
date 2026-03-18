//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_CONVEX_HULL_H
#define HELLO_MAC_CONVEX_HULL_H
#include "base_geometry/base.h"

bool clean_point_not_on_convex_hull(std::vector<Point_2> &polygon_points);

std::vector<Point_2> &calculate_convex_hull(std::vector<Point_2> &polygon_points);

inline bool convex(const Trapezoid &trapezoid) {
    auto point_a = trapezoid.left_upper;
    auto point_b = trapezoid.right_upper;
    auto point_c = trapezoid.left_lower;
    auto point_d = trapezoid.right_lower;
    auto bool_1 = Point_2::is_anticlockwise(point_a, point_d, point_c);
    auto bool_2 = Point_2::is_anticlockwise(point_d, point_c, point_b);
    auto bool_3 = Point_2::is_anticlockwise(point_c, point_b, point_a);
    auto bool_4 = Point_2::is_anticlockwise(point_b, point_a, point_d);
    if (bool_1 & bool_2 & bool_3 & bool_4) {
        return true;
    }
    return false;
}

#endif //HELLO_MAC_CONVEX_HULL_H
