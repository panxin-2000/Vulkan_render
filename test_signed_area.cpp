//
// Created by 潘鑫 on 2025/10/1.
//

#include "gtest/gtest.h"
#include "vector_signed_area.h"


// 顺时针
TEST(triangle, fsd) { {
        Point_2 ab{1, 1};
        Point_2 ac{2, 0};
        float area = ab.single_area(ac);
        EXPECT_GT(0, area);
    } {
        Point_2 ab{-10, -1};
        Point_2 ac{-2, -0};
        float area = ab.single_area(ac);
        EXPECT_GT(0, area);
    }
}

//逆时针
TEST(triangle, fdsd) { {
        Point_2 ab{2, 0};
        Point_2 ac{1, 1};
        float area = ab.single_area(ac);
        EXPECT_LT(0, area);
    } {
        Point_2 ab{1, 1};
        Point_2 ac{2, 3};
        float area = ab.single_area(ac);
        EXPECT_LT(0, area);
    }
}

/**
 * 有三个点，分别是1，1    5，5    5，1
 * 5，1 在另外两个点的下方
 */
TEST(triangle, three_point) {
    Point_2 point_a{1, 1};
    Point_2 point_b{5, 5};
    Point_2 point_c{5, 1};
    Point_2 ab = point_b - point_a;
    Point_2 ac = point_c - point_a;
    Point_2 bc = point_c - point_b;
    EXPECT_GT(0, ab.single_area(ac)); // 这里并不是为了判断等于，只是为了判断方向
    EXPECT_GT(0, ab.single_area(bc));
    EXPECT_EQ(ab.single_area(ac), ab.single_area(bc));
    // 三角形的三个点，只要第三个点在另外两个点的逆时针方向，那么这个三角形就是逆时针的三角形
    // 所以可以通过这个办法来简单的判断顺时针和逆时针，确实只有在做的时候才会更加了解具体相关的细节
}


TEST(sort, sort_segment_vector) {
    std::vector<Point_2> segments{};
    segments.push_back(Point_2{1, 1});
    segments.push_back(Point_2{5, 5});
    segments.push_back(Point_2{5, 1});
    segments.push_back(Point_2{5, 3}); // 最后这个点会消失掉
    segments.push_back(Point_2{6, 3});
    segments.push_back(Point_2{6, 4});

    std::vector<Point_2> &new_segments = calculate_convex_hull(segments);

    std::vector<Point_2> result_segments{};
    result_segments.push_back(Point_2{1, 1});
    result_segments.push_back(Point_2{5, 1});
    result_segments.push_back(Point_2{6, 3});
    result_segments.push_back(Point_2{6, 4});
    result_segments.push_back(Point_2{5, 5});
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


TEST(sort, sort_segment_vector_2) {
    std::vector<Point_2> segments{};
    segments.push_back(Point_2{1, 1});
    segments.push_back(Point_2{3, 2});
    segments.push_back(Point_2{4, 4});
    segments.push_back(Point_2{2, 5}); // 最后这个点会消失掉
    segments.push_back(Point_2{-1, 6});
    segments.push_back(Point_2{-4, 4});
    segments.push_back(Point_2{-3, 2});
    segments.push_back(Point_2{-5, 1});

    std::vector<Point_2> &result_segments = calculate_convex_hull(segments);

    std::vector<Point_2> expect_segments{};
    expect_segments.push_back(Point_2{-5, 1});
    expect_segments.push_back(Point_2{1, 1});
    expect_segments.push_back(Point_2{3, 2});
    expect_segments.push_back(Point_2{4, 4});
    expect_segments.push_back(Point_2{2, 5});
    expect_segments.push_back(Point_2{-1, 6});
    expect_segments.push_back(Point_2{-4, 4});
    // 这里的顺序变化很大的
    if (result_segments.size() == expect_segments.size()) {
        for (int i = 0; i < result_segments.size(); ++i) {
            EXPECT_EQ(result_segments.at(i), expect_segments.at(i)) << "i value: " << i
        << " result_segments x: " << result_segments.at(i).x
        << " result_segments y: " << result_segments.at(i).y
        << " expect_segments x: " << expect_segments.at(i).x
        << " expect_segments y: " << expect_segments.at(i).y
        << std::endl;
            // 这里的打印也很方便，不出现错误的时候是不需要打印的
        }
    }else {
        FAIL() << "result_segments not equal to expect_segments size.";
    }
}
