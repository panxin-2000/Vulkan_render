//
// Created by 潘鑫 on 2025/10/8.
//

#include <gtest/gtest.h>
#include "vector_signed_area.h"
#include "ear_clip.h"

TEST(ear_clip, ear_clip) {
    // 既然是测试，那么需要测试输入有什么？
    // 输入就需要一个half-edge 的结构，
    // 然后，可以是另一个结构的，比如什么呢？
    // 一系列的点，默认他们之间存在边的连接的线
    // 既然有了输入，那么输出呢？
    // 一堆的三角形
    // 三角形应该是什么结构呢？
    // 三个点组成的三角形，可以的，但是之后还需要改
    // 为什么还需要改呢？
    // 因为之后还需要按照顶点和索引的方式来确定最后给出到vulkan或者OpenGL的结构
    // 那么先不管这些，第一个问题是，如果出现了一个凹点的时候应该怎么解决呢？
    //


    std::vector<segment_vector> segments{};
    segments.push_back(segment_vector{-2, 3});
    segments.push_back(segment_vector{0, 0});
    segments.push_back(segment_vector{3, 2});
    segments.push_back(segment_vector{5, 1});
    segments.push_back(segment_vector{7, 2});
    segments.push_back(segment_vector{5, 3});
    segments.push_back(segment_vector{3, 3});
    segments.push_back(segment_vector{2, 5});
    segments.push_back(segment_vector{1, 2});
    segments.push_back(segment_vector{-1, 3});
    segments.push_back(segment_vector{0, 5});
    std::vector<segment_vector> copy_vertices;
    for (auto vertice: segments) {
        copy_vertices.push_back(vertice);
    }


    std::vector<triangle> expect_triangles{};
    std::vector<triangle> result_segments{};
    expect_triangles.push_back(triangle{{3, 2}, {5, 1}, {7, 2}});
    expect_triangles.push_back(triangle{{3, 2}, {7, 2}, {5, 3}});
    expect_triangles.push_back(triangle{{3, 2}, {5, 3}, {3, 3}});
    expect_triangles.push_back(triangle{{3, 2}, {3, 3}, {2, 5}});
    expect_triangles.push_back(triangle{{3, 2}, {2, 5}, {1, 2}});
    expect_triangles.push_back(triangle{{-1, 3}, {0, 5}, {-2, 3}});
    expect_triangles.push_back(triangle{{-1, 3}, {-2, 3}, {0, 0}});
    expect_triangles.push_back(triangle{{0, 0}, {3, 2}, {1, 2}});
    expect_triangles.push_back(triangle{{0, 0}, {1, 2}, {-1, 3}});
    binary_Tree_Node<segment_vector> *tree_vertices = nullptr;
    tree_vertices = tree_vertices->tree_insert_value(tree_vertices, segment_vector{-2, 3});
    tree_vertices = tree_vertices->tree_insert_value(tree_vertices, segment_vector{0, 0});
    tree_vertices = tree_vertices->tree_insert_value(tree_vertices, segment_vector{3, 2});
    tree_vertices = tree_vertices->tree_insert_value(tree_vertices, segment_vector{5, 1});
    tree_vertices = tree_vertices->tree_insert_value(tree_vertices, segment_vector{7, 2});
    tree_vertices = tree_vertices->tree_insert_value(tree_vertices, segment_vector{5, 3});
    tree_vertices = tree_vertices->tree_insert_value(tree_vertices, segment_vector{3, 3});
    tree_vertices = tree_vertices->tree_insert_value(tree_vertices, segment_vector{2, 5});
    tree_vertices = tree_vertices->tree_insert_value(tree_vertices, segment_vector{1, 2});
    tree_vertices = tree_vertices->tree_insert_value(tree_vertices, segment_vector{-1, 3});
    tree_vertices = tree_vertices->tree_insert_value(tree_vertices, segment_vector{0, 5});

    if (ear_clip_algorithm_no_efficient(result_segments, segments, *tree_vertices) == true &&
        result_segments.size() == expect_triangles.size()) {
        for (int i = 0; i < result_segments.size(); ++i) {
            EXPECT_EQ(result_segments.at(i), expect_triangles.at(i)) << "i value: " << i
        << std::endl;
            // 这里的打印也很方便，不出现错误的时候是不需要打印的
        }
    } else {
        FAIL() << "ear_clip_algorithm_no_efficient failed "
        << " or  result_segments.size() != expect_triangles.size()" << std::endl;
    }
    // 拿到了正确的输入的结果，只不过是强行拿到的，并不是自己手动计算处理的，所以结果必然是正确的
}
