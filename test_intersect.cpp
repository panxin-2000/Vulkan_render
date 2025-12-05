//
// Created by 潘鑫 on 2025/10/1.
//
#include "gtest/gtest.h"
#include "include/base_element/triangle.h"
#include "base_element/base.h"
#include "base_element/intersection.h"


TEST(intersect, have_intersect) {
    Point_2 point_a{1, 1};
    Point_2 point_b{5, 5};
    Point_2 point_c{5, 1};
    Point_2 point_d{1, 5};
    Segment<Point_2> segment_1{point_a, point_b};
    Segment<Point_2> segment_2{point_c, point_d};
    EXPECT_EQ(true, intersect(segment_1,segment_2));
}
