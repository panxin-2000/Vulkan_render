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




TEST(sort, sort_segment_vector) {
    std::vector<segment_vector> segments{};
    segments.push_back(segment_vector{1, 1});
    segments.push_back(segment_vector{5, 5});
    segments.push_back(segment_vector{5, 1});
    segments.push_back(segment_vector{5, 3}); // 最后这个点会消失掉
    segments.push_back(segment_vector{6, 3});
    segments.push_back(segment_vector{6, 4});

    std::vector<segment_vector> &new_segments = calculate_convex_hull(segments);

    std::vector<segment_vector> result_segments{};
    result_segments.push_back(segment_vector{1, 1});
    result_segments.push_back(segment_vector{5, 1});
    result_segments.push_back(segment_vector{6, 3});
    result_segments.push_back(segment_vector{6, 4});
    result_segments.push_back(segment_vector{5, 5});
    // 这里的顺序变化很大的

    for (int i = 0; i < new_segments.size(); ++i) {
        EXPECT_EQ(new_segments.at(i), result_segments.at(i)) << "i value: " << i
        << " new_segments x: " << new_segments.at(i).x
        << " new_segments y: " << new_segments.at(i).y
        << " result_segments x: " << result_segments.at(i).x
        << " result_segments y: " << result_segments.at(i).y
        << std::endl;
        // 这里的打印也很方便，不出现错误的时候是不需要打印的
    }
}


