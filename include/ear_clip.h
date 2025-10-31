//
// Created by 潘鑫 on 2025/10/9.
//

#ifndef EAR_CLIP_H
#define EAR_CLIP_H


#include "vector_signed_area.h"
#include "RB_tree_node.h"
#include "half_edge.h"

bool ear_clip_algorithm_no_efficient(std::vector<triangle<point_2> > &result_segments,
                                     std::vector<point_2> &new_segments,
                                     RB_Tree_Node<point_2> &tree_vertices);

bool no_point_in_line_clockwise_direction_binary(point_2 a, point_2 c,
                                                 point_2 do_not_care_point,
                                                 RB_Tree_Node<point_2> &tree_vertices_root);

template<typename T>
bool ear_clip_algorithm_half_edge(half_edge_struct<vertex_xy> &hf,
                                  T &new_segments,
                                  RB_Tree_Node<point_2> &tree_vertices) {
    if (new_segments.size() < 3) {
        return false;
    }
    // 首先拿到前三个，
    while (new_segments.size() > 3) {
        point_2 a{hf.get_vertex(new_segments.at(0)).x, hf.get_vertex(new_segments.at(0)).y};
        point_2 b{hf.get_vertex(new_segments.at(1)).x, hf.get_vertex(new_segments.at(1)).y};
        point_2 c{hf.get_vertex(new_segments.at(2)).x, hf.get_vertex(new_segments.at(2)).y};
        // 判断这三个点是顺时针还是逆时针
        if (point_2::is_anticlockwise(a, b, c) &&
            no_point_in_line_clockwise_direction_binary(a, c, b, tree_vertices)) {
            // 那么这里是逆时针,并且 所以顶点都不在 ac 的x轴范围内的点，都不在逆时针的方向上
            auto tem = hf.split_face(new_segments.at(0), new_segments.at(2));
            new_segments.at(0) = hf.get_opposite(tem);
            new_segments.erase(new_segments.begin() + 1);
        } else {
            // 将第一个点放回最后
            auto set_to_last = *new_segments.begin();
            new_segments.erase(new_segments.begin());
            new_segments.push_back(set_to_last);
        }
    }
    if (new_segments.size() == 3) {
        // 只剩三个点时候，就已经是一个三角形了

        return true;
    }
    return false;
}

#endif //EAR_CLIP_H
