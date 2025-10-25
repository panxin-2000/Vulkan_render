//
// Created by 潘鑫 on 2025/10/8.
//

#include <gtest/gtest.h>
#include "vector_signed_area.h"
#include "ear_clip.h"
#include "half_edge.h"

template<typename T>
T &init_hf(T &hf) {
    auto half_edge_index = hf.create_loop({0, 5}, {-2, 3});
    auto first_half_edge = half_edge_index;
    half_edge_index = hf.add_edge(half_edge_index, {0, 0});
    half_edge_index = hf.insert_edge(half_edge_index, {3, 2});
    half_edge_index = hf.insert_edge(half_edge_index, {5, 1});
    half_edge_index = hf.insert_edge(half_edge_index, {7, 2});
    half_edge_index = hf.insert_edge(half_edge_index, {5, 3});
    half_edge_index = hf.insert_edge(half_edge_index, {3, 3});
    half_edge_index = hf.insert_edge(half_edge_index, {2, 5});
    half_edge_index = hf.insert_edge(half_edge_index, {1, 2});
    half_edge_index = hf.insert_edge(half_edge_index, {-1, 3});
    return hf;
}

TEST(ear_clip, from_half_edge_create_loop_vertices) {
    half_edge_struct<vertex_xy> hf{};
    hf = init_hf(hf);
    face temp;
    hf.get_first_face(temp);
    auto all_edge = hf.get_all_edge_of_face(hf.get_pre(temp.bounding_half_edge));
    auto new_segments = hf.get_vertices(all_edge);

    std::vector<point_2> segments{};
    segments.push_back(point_2{-2, 3});
    segments.push_back(point_2{0, 0});
    segments.push_back(point_2{3, 2});
    segments.push_back(point_2{5, 1});
    segments.push_back(point_2{7, 2});
    segments.push_back(point_2{5, 3});
    segments.push_back(point_2{3, 3});
    segments.push_back(point_2{2, 5});
    segments.push_back(point_2{1, 2});
    segments.push_back(point_2{-1, 3});
    segments.push_back(point_2{0, 5});

    if (segments.size() == new_segments.size()) {
        for (int i = 0; i < new_segments.size(); ++i) {
            EXPECT_EQ(new_segments.at(i), segments.at(i)) << "i value: " << i
        << std::endl;
            // 这里的打印也很方便，不出现错误的时候是不需要打印的
        }
    } else {
        FAIL() << " segments.size() == new_segments.size()" << std::endl;
    }
}

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


    half_edge_struct<vertex_xy> hf{};
    hf = init_hf(hf);
    face temp;
    hf.get_first_face(temp);
    auto all_edge = hf.get_all_edge_of_face(hf.get_pre(temp.bounding_half_edge));
    auto new_segments = hf.get_vertices(all_edge);

    RB_Tree_Node<point_2> *tree_vertices = nullptr;
    for (auto new_segment: new_segments) {
        tree_vertices = tree_vertices->tree_insert_value(tree_vertices, new_segment);
    }

    std::vector<triangle> expect_triangles{};
    std::vector<triangle> result_segments{};
    expect_triangles.push_back(triangle{{3, 2}, {5, 3}, {3, 3}});
    expect_triangles.push_back(triangle{{3, 2}, {7, 2}, {5, 3}});
    expect_triangles.push_back(triangle{{0, 0}, {1, 2}, {-1, 3}});
    expect_triangles.push_back(triangle{{3, 2}, {5, 1}, {7, 2}});
    expect_triangles.push_back(triangle{{3, 2}, {2, 5}, {1, 2}});
    expect_triangles.push_back(triangle{{0, 0}, {3, 2}, {1, 2}});
    expect_triangles.push_back(triangle{{3, 2}, {3, 3}, {2, 5}});
    expect_triangles.push_back(triangle{{-1, 3}, {0, 5}, {-2, 3}});
    expect_triangles.push_back(triangle{{-1, 3}, {-2, 3}, {0, 0}});


    std::sort(expect_triangles.begin(), expect_triangles.end(), std::less<>());


    if (ear_clip_algorithm_no_efficient(result_segments, new_segments, *tree_vertices) == true &&
        result_segments.size() == expect_triangles.size()) {
        std::sort(result_segments.begin(), result_segments.end(), std::less<>());
        for (int i = 0; i < result_segments.size(); ++i) {
            EXPECT_EQ(result_segments.at(i), expect_triangles.at(i)) << "i value: " << i << std::endl;
            // 这里的打印也很方便，不出现错误的时候是不需要打印的
        }
    } else {
        FAIL() << "ear_clip_algorithm_no_efficient failed "
        << " or  result_segments.size() != expect_triangles.size()" << std::endl;
    }
    // 拿到了正确的输入的结果，只不过是强行拿到的，并不是自己手动计算处理的，所以结果必然是正确的
}


TEST(ear_clip, ear_clip_half_edge) {
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


    half_edge_struct<vertex_xy> hf{};
    hf = init_hf(hf);
    face temp;
    hf.get_first_face(temp);
    auto all_edge = hf.get_all_edge_of_face(hf.get_pre(temp.bounding_half_edge));
    auto new_segments = hf.get_vertices(all_edge);

    RB_Tree_Node<point_2> *tree_vertices = nullptr;
    for (auto new_segment: new_segments) {
        tree_vertices = tree_vertices->tree_insert_value(tree_vertices, new_segment);
    }

    std::vector<triangle> expect_triangles{};
    std::vector<triangle> result_segments{};
    expect_triangles.push_back(triangle{{0, 0}, {1, 2}, {-1, 3}});
    expect_triangles.push_back(triangle{{3, 2}, {5, 1}, {7, 2}});
    expect_triangles.push_back(triangle{{3, 2}, {7, 2}, {5, 3}});
    expect_triangles.push_back(triangle{{3, 2}, {5, 3}, {3, 3}});
    expect_triangles.push_back(triangle{{3, 2}, {3, 3}, {2, 5}});
    expect_triangles.push_back(triangle{{3, 2}, {2, 5}, {1, 2}});
    expect_triangles.push_back(triangle{{-1, 3}, {0, 5}, {-2, 3}});
    expect_triangles.push_back(triangle{{-1, 3}, {-2, 3}, {0, 0}});
    expect_triangles.push_back(triangle{{0, 0}, {3, 2}, {1, 2}});

    if (ear_clip_algorithm_half_edge(hf, all_edge, *tree_vertices) == true) {
        std::vector<triangle> result_segments{};
        auto temp_flag = hf.print_all_face_vertices(result_segments);
        if (temp_flag == true && result_segments.size() == expect_triangles.size()) {
            for (int i = 0; i < result_segments.size(); ++i) {
                EXPECT_EQ(result_segments.at(i)== expect_triangles.at(i), true) << "i value: " << i << std::endl;
                // 这里的打印也很方便，不出现错误的时候是不需要打印的
                // 比较还是有点问题，两个向量对比需要重新写一个函数，因为它们的排序可能会不一致
            }
        } else {
            FAIL() << "ear_clip_algorithm_no_efficient failed "
            << " or  result_segments.size() != expect_triangles.size()" << std::endl;
        }
    } else {
        FAIL() << "ear_clip_algorithm_half_edge return false " << std::endl;
    }
    // 拿到了正确的输入的结果，只不过是强行拿到的，并不是自己手动计算处理的，所以结果必然是正确的
}

// 如果是不带洞的，那么直接用是没有问题的，带洞的话，就稍微有点问题，不是论文中提到的办法能够直接解决的了
// 第一件事是三角形的划分结果肯定是对的，那么问题在哪里？
