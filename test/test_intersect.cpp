//
// Created by 潘鑫 on 2025/10/1.
//
#include "gtest/gtest.h"
#include "base_geometry/intersect_function.h"


TEST(intersect, have_intersect) {
    Point_2 point_a{1, 1};
    Point_2 point_b{5, 5};
    Point_2 point_c{5, 1};
    Point_2 point_d{1, 5};
    Segment<Point_2> segment_1{point_a, point_b};
    Segment<Point_2> segment_2{point_c, point_d};
    EXPECT_EQ(true, is_intersect(segment_1,segment_2));
}

TEST(AABB_bounding_box, segment) {
    Point_2 a(3, 3);
    Point_2 b(6, 6);
    AABB_min_max<Point_2> box{a, b}; {
        Segment<Point_2> temp{{2, 6}, {3, 7}};
        EXPECT_EQ(is_intersect(box, temp), false);
    } {
        Segment<Point_2> temp{{2, 5}, {4, 7}};
        EXPECT_EQ(is_intersect(box, temp), true);
    } {
        Segment<Point_2> temp{{2, 4}, {5, 7}};
        EXPECT_EQ(is_intersect(box, temp), true);
    } {
        Segment<Point_2> temp{{2, 3}, {6, 7}};
        EXPECT_EQ(is_intersect(box, temp), true);
    } {
        Segment<Point_2> temp{{2, 2}, {7, 7}};
        EXPECT_EQ(is_intersect(box, temp), true);
    } {
        Segment<Point_2> temp{{3, 2}, {7, 6}};
        EXPECT_EQ(is_intersect(box, temp), true);
    } {
        Segment<Point_2> temp{{4, 2}, {7, 5}};
        EXPECT_EQ(is_intersect(box, temp), true);
    } {
        Segment<Point_2> temp{{5, 2}, {7, 4}};
        EXPECT_EQ(is_intersect(box, temp), true);
    } {
        Segment<Point_2> temp{{6, 2}, {7, 3}};
        EXPECT_EQ(is_intersect(box, temp), false);
    } {
        const AABB_min_max<Point_3> L_box{
            {-1.07830572, 5.3555727, -0.684233069}, {1.07830572, 7.20805072, 0.684233069}
        };
        const Ray<Point_3> ray{{0, 6, 6}, {0.0386429355, 0.0563697442, -0.997661828}};
        const Ray<Point_3> new_ray{{0, 6, 6}, {0, 0, -1}};
        EXPECT_EQ(is_intersect(L_box, ray), true);
        // EXPECT_EQ(is_intersect(L_box, new_ray), true);
    } {
        Segment<Point_2> temp{{1, 3}, {4, 2}};
        EXPECT_EQ(is_intersect(box, temp), false);
    } {
        Segment<Point_2> temp{{3, 1}, {9, 4}};
        EXPECT_EQ(is_intersect(box, temp), false);
    } {
        Segment<Point_2> temp{{3, 0}, {9, 3}};
        EXPECT_EQ(is_intersect(box, temp), false);
    } {
        Segment<Point_2> temp{{0, 5}, {4, 7}};
        EXPECT_EQ(is_intersect(box, temp), false);
    } {
        Segment<Point_2> temp{{5, 7}, {8, 5}};
        EXPECT_EQ(is_intersect(box, temp), false);
    } {
        Segment<Point_2> temp{{10, 10}, {8, 8}};
        EXPECT_EQ(is_intersect(box, temp), false);
    } {
        Straight_line<Point_2> temp{{10, 10}, {8, 8}};
        EXPECT_EQ(is_intersect(box, temp), true);
    } {
        Straight_line<Point_2> temp{{10, 10}, {8, 8}};
        EXPECT_EQ(is_intersect(box, temp), true);
    } {
        Sphere<Point_2> sphere({0, 0}, 3); {
            Ray<Point_2> temp{{10, 10}, {1, 1}};
            EXPECT_EQ(is_intersect(sphere, temp), false);
        }
    } {
        Sphere<Point_2> sphere({0, 0}, 3); {
            Ray<Point_2> temp{{10, 10}, {-1, -1}};
            EXPECT_EQ(is_intersect(sphere, temp), true);
        }
    } {
        Sphere<Point_2> sphere({0, 0}, 3); {
            Ray<Point_2> temp{{5, 0}, {1, 1}};
            EXPECT_EQ(is_intersect(sphere, temp), false);
        }
    }
}

TEST(on_segment_bounding_box, on_segment_bounding_box) { {
        Point_2 a(12, 6);
        Point_2 b(17, 10);
        Point_2 c(7, 8);
        EXPECT_EQ(is_intersect(AABB_centroid<Point_2>{a, b}, c), true);
    } {
        Point_2 a(0, 2);
        Point_2 b(2, 0);
        Point_2 c(1, 1);
        EXPECT_EQ(is_intersect(AABB_centroid<Point_2>{a, b}, c), false);
    } {
        Point_2 a(3, 2);
        Point_2 b(5, 3);
        Point_2 c(2.9, 3);
        EXPECT_EQ(is_intersect(AABB_min_max<Point_2>{a, b}, c), false);
    } {
        Point_2 a(3, 2);
        Point_2 b(5, 3);
        Point_2 c(-1, 2);
        EXPECT_EQ(is_intersect(AABB_min_max<Point_2>{a, b}, c), false);
    } {
        Point_2 a(12, 6);
        Point_2 b(17, 10);
        Point_2 c(7, 8);
        EXPECT_EQ(is_intersect(AABB_min_max<Point_2>{a, b}, c), false);
    }

    EXPECT_EQ(is_intersect(Sphere<Point_2>{{0, 0}, 3}, Point_2{2, 2}), true);
    EXPECT_EQ(is_intersect(Sphere<Point_2>{{0, 0}, 3}, Point_2{4, 2}), false);
    EXPECT_EQ(is_intersect(Sphere<Point_3>{{0, 0,0}, 3}, Point_3{2, 2,0}), true);
    EXPECT_EQ(is_intersect(Sphere<Point_3>{{0, 0,0}, 3}, Point_3{2, 2,2}), false);
    EXPECT_EQ(is_intersect(Sphere<Point_3>{{0, 0,0}, 3}, Point_3{4, 0,2}), false);
}


TEST(AABB_bounding_box, have_intersect_axis) {
    EXPECT_EQ(have_intersect_axis( 2,4,5,6 ), false);
    EXPECT_EQ(have_intersect_axis( 2,5,4,6 ), true);
    EXPECT_EQ(have_intersect_axis( 2,7,4,6 ), true);
    EXPECT_EQ(have_intersect_axis( -1,-7,4,6 ), false);
    EXPECT_EQ(have_intersect_axis( -1,-7,-4,6 ), true);
    EXPECT_EQ(have_intersect_axis( -1,-7,4,-6 ), true);

    Point_2 a(1, 1);
    Point_2 b(4, 3);
    AABB_min_max<Point_2> box{a, b};
    EXPECT_EQ(is_intersect(box, Ray<Point_2> {{0, 0}, {1, 2}}), true);
    EXPECT_EQ(is_intersect(box, Ray<Point_2> {{0, 2}, {1, 1}}), true);
    EXPECT_EQ(is_intersect(box, Ray<Point_2> {{0, 2}, {1, 1.001}}), false);
    EXPECT_EQ(is_intersect(box, Ray<Point_2> {{0, 2}, {1, -1}}), true);
    EXPECT_EQ(is_intersect(box, Ray<Point_2> {{0, 2}, {1, -1.001}}), false);
    EXPECT_EQ(is_intersect(box, Ray<Point_2> {{0, 0}, {1, 4}}), false);
    EXPECT_EQ(is_intersect(box, Ray<Point_2> {{5, 5}, {1, 1}}), false);
    EXPECT_EQ(is_intersect(box, Ray<Point_2> {{5, 2}, {-1, -1}}), true);
    EXPECT_EQ(is_intersect(box, Ray<Point_2> {{5, 2}, {-1, -1.001}}), false);
    EXPECT_EQ(is_intersect(box, Ray<Point_2> {{2, 2}, {1, -1}}), true);
    EXPECT_EQ(is_intersect(box, Ray<Point_2> {{2, 2}, {1, 1}}), true);
    EXPECT_EQ(is_intersect(box, Ray<Point_2> {{2, 2}, {-1, 1}}), true);
    EXPECT_EQ(is_intersect(box, Ray<Point_2> {{2, 2}, {-1, -1}}), true);

    AABB_min_max<Point_2> box_2{{1, 0}, {4, 0}};

    // 添加了一系列退化的情况
    EXPECT_EQ(is_intersect(box_2, Ray<Point_2> {{0, 0}, {1, 0}}), true);
    EXPECT_EQ(is_intersect(box_2, Ray<Point_2> {{0, 0}, {-1, 0}}), false);
    EXPECT_EQ(is_intersect(box_2, Ray<Point_2> {{0, 1}, {1, 0}}), false);
    EXPECT_EQ(is_intersect(box_2, Ray<Point_2> {{0, 1}, {-1, 0}}), false);
    EXPECT_EQ(is_intersect(box_2, Ray<Point_2> {{0, 1}, {0, 1}}), false);
    EXPECT_EQ(is_intersect(box_2, Ray<Point_2> {{0, 1}, {0, -1}}), false);
    EXPECT_EQ(is_intersect(box_2, Ray<Point_2> {{2, 1}, {0, 1}}), false);
    EXPECT_EQ(is_intersect(box_2, Ray<Point_2> {{2, 1}, {0, -1}}), true);


    Point_3 a_3d(1, 1, 0);
    Point_3 b_3d(4, 3, 0);
    AABB_min_max<Point_3> box_3d{a, b};
    EXPECT_EQ(is_intersect(box_3d, Ray<Point_3> {{0, 0,0}, {1, 2,0}}), true);
    EXPECT_EQ(is_intersect(box_3d, Ray<Point_3> {{0, 2,0}, {1, 1,0}}), true);
    EXPECT_EQ(is_intersect(box_3d, Ray<Point_3> {{0, 2,0}, {1, 1.001,0}}), false);
    EXPECT_EQ(is_intersect(box_3d, Ray<Point_3> {{0, 2,0}, {1, -1,0}}), true);
    EXPECT_EQ(is_intersect(box_3d, Ray<Point_3> {{0, 2,0}, {1, -1.001,0}}), false);
    EXPECT_EQ(is_intersect(box_3d, Ray<Point_3> {{0, 0,0}, {1, 4,0}}), false);
    EXPECT_EQ(is_intersect(box_3d, Ray<Point_3> {{5, 5,0}, {1, 1,0}}), false);
    EXPECT_EQ(is_intersect(box_3d, Ray<Point_3> {{5, 2,0}, {-1, -1,0}}), true);
    EXPECT_EQ(is_intersect(box_3d, Ray<Point_3> {{5, 2,0}, {-1, -1.001,0}}), false);
    EXPECT_EQ(is_intersect(box_3d, Ray<Point_3> {{2, 2,0}, {1, -1,0}}), true);
    EXPECT_EQ(is_intersect(box_3d, Ray<Point_3> {{2, 2,0}, {1, 1,0}}), true);
    EXPECT_EQ(is_intersect(box_3d, Ray<Point_3> {{2, 2,0}, {-1, 1,0}}), true);
    EXPECT_EQ(is_intersect(box_3d, Ray<Point_3> {{2, 2,0}, {-1, -1,0}}), true);
}


TEST(distance, point_to_segment) { {
        Segment<Point_2> temp{{0, 0}, {2, 0}};
        EXPECT_EQ(distance(temp, {1, 1} ), 1);
        EXPECT_EQ(distance(temp,{1,0}), 0);
        EXPECT_EQ(distance(temp,{0,0}), 0);
        EXPECT_EQ(distance(temp,{2,0}), 0);
        EXPECT_EQ(distance(temp,{2,1}), 1);
        EXPECT_EQ(distance(temp,{3,1}), 2);
        EXPECT_EQ(distance(temp,{-1,1}), 2);
    } {
        Segment<Point_3> temp{{0, 0, 0}, {2, 0, 0}};
        EXPECT_EQ(distance(temp, {1, 1,0} ), 1);
        EXPECT_EQ(distance(temp,{1,0,0}), 0);
        EXPECT_EQ(distance(temp,{0,0,0}), 0);
        EXPECT_EQ(distance(temp,{2,0,0}), 0);
        EXPECT_EQ(distance(temp,{2,1,0}), 1);
        EXPECT_EQ(distance(temp,{3,1,0}), 2);
        EXPECT_EQ(distance(temp,{-1,1,0}), 2);
    }
}


TEST(distance, point_to_ray) { {
        Ray<Point_2> temp{{0, 0}, {2, 0}};
        EXPECT_EQ(distance(temp, {1, 1} ), 1);
        EXPECT_EQ(distance(temp,{1,0}), 0);
        EXPECT_EQ(distance(temp,{0,0}), 0);
        EXPECT_EQ(distance(temp,{2,0}), 0);
        EXPECT_EQ(distance(temp,{2,1}), 1);
        EXPECT_EQ(distance(temp,{3,1}), 1);
        EXPECT_EQ(distance(temp,{-1,1}), 2);
    } {
        Ray<Point_3> temp{{0, 0, 0}, {2, 0, 0}};
        EXPECT_EQ(distance(temp, {1, 1,0} ), 1);
        EXPECT_EQ(distance(temp,{1,0,0}), 0);
        EXPECT_EQ(distance(temp,{0,0,0}), 0);
        EXPECT_EQ(distance(temp,{2,0,0}), 0);
        EXPECT_EQ(distance(temp,{2,1,0}), 1);
        EXPECT_EQ(distance(temp,{3,1,0}), 1);
        EXPECT_EQ(distance(temp,{-1,1,0}), 2);
    }
}

TEST(distance, point_to_straight_line) { {
        Straight_line<Point_2> temp{{0, 0}, {2, 0}};
        EXPECT_EQ(distance(temp, {1, 1} ), 1);
        EXPECT_EQ(distance(temp,{1,0}), 0);
        EXPECT_EQ(distance(temp,{0,0}), 0);
        EXPECT_EQ(distance(temp,{2,0}), 0);
        EXPECT_EQ(distance(temp,{2,1}), 1);
        EXPECT_EQ(distance(temp,{3,1}), 1);
        EXPECT_EQ(distance(temp,{-1,1}), 1);
    } {
        Straight_line<Point_3> temp{{0, 0, 0}, {2, 0, 0}};
        EXPECT_EQ(distance(temp, {1, 1,0} ), 1);
        EXPECT_EQ(distance(temp,{1,0,0}), 0);
        EXPECT_EQ(distance(temp,{0,0,0}), 0);
        EXPECT_EQ(distance(temp,{2,0,0}), 0);
        EXPECT_EQ(distance(temp,{2,1,0}), 1);
        EXPECT_EQ(distance(temp,{3,1,0}), 1);
        EXPECT_EQ(distance(temp,{-1,1,0}), 1);
    }
}

TEST(distance, point_to_plane) {
    Plane<Point_3> temp{{0, 0, 0}, {0, 0, 1}};
    EXPECT_EQ(distance(temp, {1, 1,0} ), 0);
    EXPECT_EQ(distance(temp,{1,0,0}), 0);
    EXPECT_EQ(distance(temp,{0,0,0}), 0);
    EXPECT_EQ(distance(temp,{2,0,0}), 0);
    EXPECT_EQ(distance(temp,{2,1,0}), 0);
    EXPECT_EQ(distance(temp,{3,1,0}), 0);
    EXPECT_EQ(distance(temp,{-1,1,0}), 0);

    EXPECT_EQ(distance(temp, {1, 1,1} ), 1);
    EXPECT_EQ(distance(temp,{2,-2,-2}), 4);
}

TEST(distance, point_to_AABB) {
    auto a = std::clamp(10, 2, 5);
    auto b = clamp(Point_2{1, 23}, Point_2{2, 3}, Point_2{5, 6});
    auto c = clamp(Point_3{1, 23, 9}, Point_3{2, 3, 3}, Point_3{5, 6, 15});

    AABB_min_max<Point_3> temp{{0, 0, 0}, {1, 1, 1}};
    EXPECT_EQ(distance(temp, {1, 1,0} ), 0);
    EXPECT_EQ(distance(temp,{1,0,0}), 0);
    EXPECT_EQ(distance(temp,{0,0,0}), 0);
    EXPECT_EQ(distance(temp,{2,0,0}), 1);
    EXPECT_EQ(distance(temp,{2,1,0}), 1);
    EXPECT_EQ(distance(temp,{3,1,0}), 4);
    EXPECT_EQ(distance(temp,{-1,1,0}), 1);

    EXPECT_EQ(distance(temp, {1, 1,1} ), 0);
    EXPECT_EQ(distance(temp,{2,-2,-2}), 1 + 4 + 4);
}

TEST(distance, point_to_Sphere) { {
        Sphere temp{Point_2{0, 0}, 2};
        EXPECT_EQ(distance(temp, {1, 1} ), 0);
        EXPECT_EQ(distance(temp, {-1, 1} ), 0);
        EXPECT_EQ(distance(temp, {-1, -1} ), 0);
        EXPECT_EQ(distance(temp, {1, -1} ), 0);
        EXPECT_EQ(distance(temp, {0, 0} ), 0);
        // 下面一行判断相等的时候需要注意精度问题
        EXPECT_EQ(distance(temp, {2, 2} ), (sqrt(2.0f) * 2.0f - 2.0f) * (sqrt(2.0f) * 2.0f - 2.0f));
    } {
        Sphere temp{Point_3{0, 0, 0}, 2};
        EXPECT_EQ(distance(temp, {1, 1,0} ), 0);
        EXPECT_EQ(distance(temp, {-1, 1,0} ), 0);
        EXPECT_EQ(distance(temp, {-1, -1,0} ), 0);
        EXPECT_EQ(distance(temp, {1, -1,0} ), 0);
        EXPECT_EQ(distance(temp, {0, 0,0} ), 0);
        // 下面一行判断相等的时候需要注意精度问题
        EXPECT_EQ(distance(temp, {2, 2,0} ), (sqrt(2.0f) * 2.0f - 2.0f) * (sqrt(2.0f) * 2.0f - 2.0f));
    }
}

TEST(distance, Barycentric_coordinates) {
    Triangle<Point_2> a{{0, 0}, {4, 0}, {2, 4}};
    // 给出顶点，测试 Voronoi 区域的类型
    auto ab    = distance(a, {-1, 0});
    auto bc    = distance(a, {5, 0});
    auto ac    = distance(a, {2, 5});
    auto acf   = distance(a, {3, 3});
    auto acff  = distance(a, {2, -1});
    auto acsf  = distance(a, {1, 3});
    auto actsf = distance(a, {2, 2});

    int d = 0;
}

TEST(intersect, point_in_OBB) {
    OBB_2D<Point_2> temp{
        {0, 0},
        {sqrt(2.0f) / 2.0f, sqrt(2.0f) / 2.0f},
        {sqrt(2.0f) / 2.0f, -sqrt(2.0f) / 2.0f},
        {2, 2}
    };
    EXPECT_EQ(is_intersect(temp, {1, 1} ), true);
    EXPECT_EQ(is_intersect(temp, {-1, 1} ), true);
    EXPECT_EQ(is_intersect(temp, {-1, -1} ), true);
    EXPECT_EQ(is_intersect(temp, {1, -1} ), true);
    EXPECT_EQ(is_intersect(temp, {0, 0} ), true);
    EXPECT_EQ(is_intersect(temp, {sqrt(2.0f) * 2.0f, 0} ), true);
    EXPECT_EQ(is_intersect(temp, {0,sqrt(2.0f) * 2.0f} ), true);
    EXPECT_EQ(is_intersect(temp, {sqrt(2.0f) * 2.0f + 0.001f, 0} ), false);
    EXPECT_EQ(is_intersect(temp, {0,sqrt(2.0f) * 2.0f + 0.001f } ), false);
    EXPECT_EQ(is_intersect(temp, {1, 2} ), false);
    EXPECT_EQ(is_intersect(temp, {2, 1} ), false);

    EXPECT_EQ(distance(temp, {1, 1} ), 0.0f);
    EXPECT_EQ(distance(temp, {-1, 1} ), 0.0f);
    EXPECT_EQ(distance(temp, {-1, -1} ), 0.0f);
    EXPECT_EQ(distance(temp, {1, -1} ), 0.0f);
    EXPECT_EQ(distance(temp, {0, 0} ), 0.0f);
    EXPECT_EQ(distance(temp, {sqrt(2.0f) * 2.0f, 0} ), 0.0f);
    EXPECT_EQ(distance(temp, {0,sqrt(2.0f) * 2.0f} ), 0.0f);
    EXPECT_NEAR(distance(temp, {sqrt(2.0f) * 2.0f + 1.0f, 0} ), 1.0f, 1e-6);
    EXPECT_NEAR(distance(temp, {0,sqrt(2.0f) * 2.0f + 1.0f } ), 1.0f, 1e-6);
    EXPECT_NEAR(distance(temp, {2, 2} ),
                (sqrt(2.0f) * 2.0f - 2.0f) * (sqrt(2.0f) * 2.0f - 2.0f),
                1e-6);
    // EXPECT_EQ(intersect(temp, {2, 1} ), false);
}
