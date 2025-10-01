//
// Created by 潘鑫 on 2025/10/1.
//

#include "gtest/gtest.h"
#include "vector_signed_area.h"


// 顺时针
TEST(triangle, fsd) { {
    segment_vector ab{1, 1};
    segment_vector ac{2, 0};
    float area = ab.single_area(ac);
    EXPECT_GT(0, area);
} {
    segment_vector ab{-10, -1};
    segment_vector ac{-2, -0};
    float area = ab.single_area(ac);
    EXPECT_GT(0, area);
}
}

//逆时针
TEST(triangle, fdsd) { {
    segment_vector ab{2, 0};
    segment_vector ac{1, 1};
    float area = ab.single_area(ac);
    EXPECT_LT(0, area);
} {
    segment_vector ab{1, 1};
    segment_vector ac{2, 3};
    float area = ab.single_area(ac);
    EXPECT_LT(0, area);
}
}

/**
 * 有三个点，分别是1，1    5，5    5，1
 * 5，1 在另外两个点的下方
 */
TEST(triangle, three_point) {
    segment_vector point_a{1, 1};
    segment_vector point_b{5, 5};
    segment_vector point_c{5, 1};
    segment_vector ab = point_b - point_a;
    segment_vector ac = point_c - point_a;
    segment_vector bc = point_c - point_b;
    EXPECT_GT(0, ab.single_area(ac)); // 这里并不是为了判断等于，只是为了判断方向
    EXPECT_GT(0, ab.single_area(bc));
    EXPECT_EQ(ab.single_area(ac), ab.single_area(bc));
    // 三角形的三个点，只要第三个点在另外两个点的逆时针方向，那么这个三角形就是逆时针的三角形
    // 所以可以通过这个办法来简单的判断顺时针和逆时针，确实只有在做的时候才会更加了解具体相关的细节
}