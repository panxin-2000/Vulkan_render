//
// Created by 潘鑫 on 2025/11/12.
//
#include "gtest/gtest.h"

#include "test_delaunay_triangulation.h"

TEST(delaunay_triangulation, delaunay_triangulation) {
    std::vector<point_2> input_points;
    input_points.push_back({7, 8});
    input_points.push_back({12, 8});
    input_points.push_back({10, 6});

    auto hf = delaunay_triangulation::delaunay_triangulation(input_points);

    EXPECT_EQ(1, 1);
}
