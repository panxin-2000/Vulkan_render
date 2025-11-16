//
// Created by 潘鑫 on 2025/10/8.
//
#include "ear_clip.h"

#include "tree_function.h"
/**
 * 所以顶点都不在 ac 的x轴范围内的点，都不在逆时针的方向上
 * @param a
 * @param C
 * @param tree_vertices
 * @return
 */
bool no_point_in_line_clockwise_direction(point_2 a, point_2 C,
                                          RB_Tree_Node<point_2> &tree_vertices) {
}

/**
 * 所以顶点都不在 ac 的x轴范围内的点，都不在逆时针的方向上
 * @param a
 * @param C
 * @param tree_vertices
 * @return
 */
bool no_point_in_line_clockwise_direction_no_efficient(point_2 a, point_2 c,
                                                       point_2 do_not_care_point,
                                                       std::vector<point_2> &tree_vertices) {
    for (auto tree_vertice: tree_vertices) {
        if ((tree_vertice.x > a.x && tree_vertice.x < c.x && !(do_not_care_point == tree_vertice)) ||
            (tree_vertice.x < a.x && tree_vertice.x > c.x && !(do_not_care_point == tree_vertice))) {
            point_2 b = tree_vertice;
            if (point_2::anticlockwise::counterclockwise != point_2::is_anticlockwise(a, b, c)) {
                return false;
            }
        }
    }
    return true;
}


bool no_point_in_line_clockwise_direction_binary(point_2 a, point_2 b,
                                                 point_2 do_not_care_point,
                                                 RB_Tree_Node<point_2> &tree_vertices_root) {
    auto vertices = find_interval(&tree_vertices_root, a, b);
    for (auto p_vertice: vertices) {
        auto tree_vertice = p_vertice->data;
        if ((tree_vertice.x > a.x && tree_vertice.x < b.x && !(do_not_care_point == tree_vertice)) ||
            (tree_vertice.x < a.x && tree_vertice.x > b.x && !(do_not_care_point == tree_vertice))) {
            point_2 c = tree_vertice;
            if (point_2::anticlockwise::counterclockwise !=point_2::is_anticlockwise(a, b, c)) {
                return false;
            }
        }
    }
    return true;
}

bool ear_clip_algorithm(std::vector<point_2> &new_segments,
                        RB_Tree_Node<point_2> &tree_vertices) {
    // 首先拿到前三个，
    while (new_segments.size() >= 3) {
        point_2 a = new_segments.at(new_segments.size() - 3);
        point_2 b = new_segments.at(new_segments.size() - 2);
        point_2 c = new_segments.at(new_segments.size() - 1);
        // 判断这三个点是顺时针还是逆时针
        if (point_2::is_anticlockwise(a, b, c) == point_2::anticlockwise::counterclockwise
            && no_point_in_line_clockwise_direction(a, c, tree_vertices)) {
            // 那么这里是逆时针,并且 所以顶点都不在 ac 的x轴范围内的点，都不在逆时针的方向上
            new_segments.erase(new_segments.begin() + 1);
        } else {
            // 需要将这三个点作为一个三角形进行输出
            point_2 set_to_last = *new_segments.begin();
            new_segments.erase(new_segments.begin());
            new_segments.push_back(set_to_last);
        }
    }
}

bool ear_clip_algorithm_no_efficient(std::vector<triangle<point_2> > &result_segments,
                                     std::vector<point_2> &new_segments,
                                     RB_Tree_Node<point_2> &tree_vertices) {
    if (new_segments.size() < 3) {
        return false;
    }
    // 首先拿到前三个，
    while (new_segments.size() > 3) {
        point_2 a = new_segments.at(0);
        point_2 b = new_segments.at(1);
        point_2 c = new_segments.at(2);
        // 判断这三个点是顺时针还是逆时针
        if (point_2::is_anticlockwise(a, b, c) == point_2::anticlockwise::counterclockwise
            && no_point_in_line_clockwise_direction_binary(a, c, b, tree_vertices)) {
            // 那么这里是逆时针,并且 所以顶点都不在 ac 的x轴范围内的点，都不在逆时针的方向上
            new_segments.erase(new_segments.begin() + 1);
            triangle<point_2> t{{a.x, a.y}, {b.x, b.y}, {c.x, c.y}};
            result_segments.push_back(t);
            // 应该是需要删除的，之后我看看，怎么写一个需要删除的例子
            // tree_vertices.delete_node_from_binary_search_tree(&tree_vertices,
            //                                                   tree_vertices.tree_find_value(
            //                                                       &tree_vertices, new_segments.at(2)));
        } else {
            // 需要将这三个点作为一个三角形进行输出
            point_2 set_to_last = *new_segments.begin();
            new_segments.erase(new_segments.begin());
            new_segments.push_back(set_to_last);
        }
    }
    if (new_segments.size() == 3) {
        // 清楚全部的内容，
        // 并进行三角化
        // 得到相应的结果
        triangle<point_2> t{
            {new_segments.at(0).x, new_segments.at(0).y},
            {new_segments.at(1).x, new_segments.at(1).y},
            {new_segments.at(2).x, new_segments.at(2).y}
        };
        result_segments.push_back(t);
        return true;
    }
    return false;
}
