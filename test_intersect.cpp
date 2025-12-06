//
// Created by 潘鑫 on 2025/10/1.
//
#include "gtest/gtest.h"
#include "include/base_element/geometry/triangle.h"
#include "base_element/base.h"


TEST(intersect, have_intersect) {
    Point_2 point_a{1, 1};
    Point_2 point_b{5, 5};
    Point_2 point_c{5, 1};
    Point_2 point_d{1, 5};
    Segment<Point_2> segment_1{point_a, point_b};
    Segment<Point_2> segment_2{point_c, point_d};
    EXPECT_EQ(true, intersect(segment_1,segment_2));
}

TEST(AABB_bounding_box, segment) {
    Point_2 a(3, 3);
    Point_2 b(6, 6);
    AABB_min_max<Point_2> box{a, b}; {
        Segment<Point_2> temp{{2, 6}, {3, 7}};
        EXPECT_EQ(intersect(box, temp), false);
    } {
        Segment<Point_2> temp{{2, 5}, {4, 7}};
        EXPECT_EQ(intersect(box, temp), true);
    } {
        Segment<Point_2> temp{{2, 4}, {5, 7}};
        EXPECT_EQ(intersect(box, temp), true);
    } {
        Segment<Point_2> temp{{2, 3}, {6, 7}};
        EXPECT_EQ(intersect(box, temp), true);
    } {
        Segment<Point_2> temp{{2, 2}, {7, 7}};
        EXPECT_EQ(intersect(box, temp), true);
    } {
        Segment<Point_2> temp{{3, 2}, {7, 6}};
        EXPECT_EQ(intersect(box, temp), true);
    } {
        Segment<Point_2> temp{{4, 2}, {7, 5}};
        EXPECT_EQ(intersect(box, temp), true);
    } {
        Segment<Point_2> temp{{5, 2}, {7, 4}};
        EXPECT_EQ(intersect(box, temp), true);
    } {
        Segment<Point_2> temp{{6, 2}, {7, 3}};
        EXPECT_EQ(intersect(box, temp), false);
    }

    //
    {
        Segment<Point_2> temp{{1, 3}, {4, 2}};
        EXPECT_EQ(intersect(box, temp), false);
    } {
        Segment<Point_2> temp{{3, 1}, {9, 4}};
        EXPECT_EQ(intersect(box, temp), false);
    } {
        Segment<Point_2> temp{{3, 0}, {9, 3}};
        EXPECT_EQ(intersect(box, temp), false);
    } {
        Segment<Point_2> temp{{0, 5}, {4, 7}};
        EXPECT_EQ(intersect(box, temp), false);
    } {
        Segment<Point_2> temp{{5, 7}, {8, 5}};
        EXPECT_EQ(intersect(box, temp), false);
    } {
        Segment<Point_2> temp{{10, 10}, {8, 8}};
        EXPECT_EQ(intersect(box, temp), false);
    } {
        Straight_line<Point_2> temp{{10, 10}, {8, 8}};
        EXPECT_EQ(intersect(box, temp), true);
    }
}

TEST(on_segment_bounding_box, on_segment_bounding_box) { {
        Point_2 a(12, 6);
        Point_2 b(17, 10);
        Point_2 c(7, 8);
        EXPECT_EQ(intersect(AABB_centroid<Point_2>{a, b}, c), false);
    } {
        Point_2 a(0, 2);
        Point_2 b(2, 0);
        Point_2 c(1, 1);
        EXPECT_EQ(intersect(AABB_centroid<Point_2>{a, b}, c), true);
    } {
        Point_2 a(3, 2);
        Point_2 b(5, 3);
        Point_2 c(2.9, 3);
        EXPECT_EQ(intersect(AABB_min_max<Point_2>{a, b}, c), false);
    } {
        Point_2 a(3, 2);
        Point_2 b(5, 3);
        Point_2 c(-1, 2);
        EXPECT_EQ(intersect(AABB_min_max<Point_2>{a, b}, c), false);
    } {
        Point_2 a(12, 6);
        Point_2 b(17, 10);
        Point_2 c(7, 8);
        EXPECT_EQ(intersect(AABB_min_max<Point_2>{a, b}, c), false);
    }

    EXPECT_EQ(intersect(Sphere<Point_2>{{0, 0}, 3}, Point_2{2, 2}), true);
    EXPECT_EQ(intersect(Sphere<Point_2>{{0, 0}, 3}, Point_2{4, 2}), false);
    EXPECT_EQ(intersect(Sphere<Point_3>{{0, 0,0}, 3}, Point_3{2, 2,0}), true);
    EXPECT_EQ(intersect(Sphere<Point_3>{{0, 0,0}, 3}, Point_3{2, 2,2}), false);
    EXPECT_EQ(intersect(Sphere<Point_3>{{0, 0,0}, 3}, Point_3{4, 0,2}), false);
}
