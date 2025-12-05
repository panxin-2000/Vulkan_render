//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_CONVEX_HULL_H
#define HELLO_MAC_CONVEX_HULL_H
#include "base_element/base.h"

bool clean_point_not_on_convex_hull(std::vector<Point_2> &polygon_points);

std::vector<Point_2> &calculate_convex_hull(std::vector<Point_2> &polygon_points);


#endif //HELLO_MAC_CONVEX_HULL_H
