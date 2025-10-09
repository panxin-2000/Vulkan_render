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

    ear_clip_algorithm_no_efficient(segments, copy_vertices);
}
