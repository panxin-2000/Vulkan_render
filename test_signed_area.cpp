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

    // 需要首先找到一个y轴最小的，如果有y轴相同的内容，那么就选择其中x轴最小的
    // 将这个点从整个vector中移出，
    // 之后呢？然后将之后的vector中的内容全部从排序，怎么排序呢？然后按照角度排序
    // 排序完成之后，按照什么样的方式计算的呢？
    // 如果新的点，在原本的两个点的顺时针的方向，那么就将中间的点移除
    // 之后再重新计算包含最新点的三个点，如果包含，再次移除第二个，并重新计算
    // 直到是在逆时针的方向的时候，再将

    struct {
        bool operator()(segment_vector a, segment_vector b) const {
            if (a.y < b.y) {
                return true;
            } else if (a.y > b.y) {
                return false;
            } else if (a.y == b.y) {
                if (a.x < b.x) {
                    return true;
                } else {
                    return false;
                }
            }
        }
    } customLess;

    std::vector<segment_vector>::iterator current_min = std::min_element(segments.begin(), segments.end(), customLess);

    segment_vector min = *current_min;
    segments.erase(current_min);
    // struct {
    //     bool operator()(segment_vector a, segment_vector b) const {
    //         segment_vector new_a = a - min;
    //         segment_vector new_b = b - min;
    //         if (std::asin(new_a.y / new_a.x) < std::asin(new_b.y / new_b.x)) {
    //             return true;
    //         } else {
    //             return false;
    //         }
    //     }
    // } custom_angle_Less;
    // 上面的写法是不行的，下面的写法是可以的，问题在于下面的多了这样的一个内容 [min]

    std::sort(segments.begin(), segments.end(),
              [min](segment_vector a, segment_vector b) {
                  segment_vector new_a = a - min;
                  segment_vector new_b = b - min;
                  if (std::asin(new_a.y / new_a.x) < std::asin(new_b.y / new_b.x)) {
                      return true;
                  } else {
                      return false;
                  }
              });
    // 上面的算出的结果是自己想要的，其实上面也应该有一个添加一个test去验证的
    // 既然暂时没有做，等做完之后一起全部简单的重构一遍就好，进行功能的划分


    std::vector<segment_vector> new_segments{};
    new_segments.push_back(min);
    // new_segments.push_back(*segments.begin());
    // segments.erase(segments.begin);
    // 上面这两行是可以被注释的，因为只是被增加了，其他并没有问题

    for (auto segment: segments) {
        new_segments.push_back(segment);
        // segments.erase(segments.begin);  // 这里其实应该也并不应该存在，因上面for导致的是有问题的
        convex_hull(new_segments);
    }

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

bool convex_hull(std::vector<segment_vector> &new_segments) {
    // 这里需要做什么呢？
    // 判断长度
    if (new_segments.size() >= 3) {
        segment_vector a = new_segments.at(new_segments.size() - 3);
        segment_vector b = new_segments.at(new_segments.size() - 2);
        segment_vector c = new_segments.at(new_segments.size() - 1);

        segment_vector ab = b - a;
        segment_vector ac = c - a;
        float area = ab.single_area(ac);
        if (area >= 0) {
            // 那么这里是逆时针
            return true;
        } else {
            // 如果是顺时针，那么就需要删除倒数第二个，然后重新计算
            // 重新计算需要重新调用这个函数本身
            // 参数是什么呢？只需要一个向量就好
            new_segments.erase(new_segments.end() - 2);
            return convex_hull(new_segments);
        }
    } else {
        return false;
    }
}
