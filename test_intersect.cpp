//
// Created by 潘鑫 on 2025/10/1.
//
#include "gtest/gtest.h"
#include "vector_signed_area.h"


TEST(intersect, have_intersect) {
    point_2 point_a{1, 1};
    point_2 point_b{5, 5};
    point_2 point_c{5, 1};
    point_2 point_d{1, 5};
    segment_position segment_1{point_a, point_b};
    segment_position segment_2{point_c, point_d};
    EXPECT_EQ(true, segment_1.intersection(segment_2));

}
