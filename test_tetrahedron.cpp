//
// Created by 潘鑫 on 2025/10/24.
//

#include "half_edge.h"
#include <gtest/gtest.h>

TEST(tetrahedron, init_tetrahedron) {
    half_edge_struct<vertex_xyz> hf;
    auto b_half_edge_index = hf.create_loop({-1, 1, 0}, {0, -1, 0});
    auto c_half_edge_index = hf.add_edge(b_half_edge_index, {0, 0, 2});
    auto d_half_edge_index = hf.add_edge(hf.get_opposite_edge_index(hf.get_pre_edge_index(c_half_edge_index)), {1, 1, 2});
    hf.split_face(hf.get_opposite_edge_index(hf.get_pre_edge_index(d_half_edge_index)), hf.get_opposite_edge_index(c_half_edge_index));

    std::vector<Triangle<point_3> > expect_triangles{};
    std::vector<Triangle<point_3> > result_segments{};
    expect_triangles.push_back(Triangle<point_3>{{-1, 1, 0}, {0, -1, 0}, {0, 0, 2}}); // blue
    expect_triangles.push_back(Triangle<point_3>{{0, 0, 2}, {0, -1, 0}, {1, 1, 2}});  // yellow
    expect_triangles.push_back(Triangle<point_3>{{-1, 1, 0}, {0, 0, 2}, {1, 1, 2}});  //  green
    expect_triangles.push_back(Triangle<point_3>{{-1, 1, 0}, {1, 1, 2}, {0, -1, 0}}); // last
    std::sort(expect_triangles.begin(), expect_triangles.end(), std::less<>());

    auto temp_flag = hf.print_all_face_vertices(result_segments, false);
    if (temp_flag == true && result_segments.size() == expect_triangles.size()) {
        std::sort(result_segments.begin(), result_segments.end(), std::less<>());
        for (int i = 0; i < result_segments.size(); ++i) {
            EXPECT_EQ(result_segments.at(i)== expect_triangles.at(i), true) << "i value: " << i << std::endl;
        }
    } else {
        FAIL() << "  result_segments.size() != expect_triangles.size()" << std::endl;
    }


    // 连接应该是连接起来了吧，估计还是有洞
    // 如果两个边的面，都是属于有洞的那一面的话，那么其实应该都转换为每一洞的那一面，但是也不对啊？
    // 因为是四面体，所以可以不用区分，投影之后，背面投影之后就是逆时针
    // 结果是对的，验证了结果

    // 然后这个时候你就应该发现了问题，那就是如何把这个图形封闭起来？
    // 如果是创建的话，那么就需要找到哪些面是hole,然后，将hole中的封闭起来，
    // 需要判断什么？判断两个点是否存在连接， 这个算法应该是可以使用ear clip的，进行三角形triangulation，
    // 但是因为是立体的，所以需要考虑一下凸包的问题，ear clip 也需要是凸的时候才能够进行
}
