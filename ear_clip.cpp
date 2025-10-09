//
// Created by 潘鑫 on 2025/10/8.
//
#include "include/ear_clip.h"

/**
 * 所以顶点都不在 ac 的x轴范围内的点，都不在逆时针的方向上
 * @param a
 * @param C
 * @param tree_vertices
 * @return
 */
bool no_point_in_line_clockwise_direction(segment_vector a, segment_vector C,
                                          RB_Tree_Node<segment_vector> &tree_vertices) {
}

/**
 * 所以顶点都不在 ac 的x轴范围内的点，都不在逆时针的方向上
 * @param a
 * @param C
 * @param tree_vertices
 * @return
 */
bool no_point_in_line_clockwise_direction_no_efficient(segment_vector a, segment_vector c,
                                                       segment_vector do_not_care_point,
                                                       std::vector<segment_vector> &tree_vertices) {
    for (auto tree_vertice: tree_vertices) {
        if ((tree_vertice.x > a.x && tree_vertice.x < c.x && !(do_not_care_point == tree_vertice)) ||
            (tree_vertice.x < a.x && tree_vertice.x > c.x && !(do_not_care_point == tree_vertice))) {
            segment_vector b = tree_vertice;
            segment_vector ab = b - a;
            segment_vector ac = c - a;
            float area = ac.single_area(ab);
            if (area <= 0) {
                return false;
            }
        }
    }
    return true;
}

bool ear_clip_algorithm(std::vector<segment_vector> &new_segments,
                        RB_Tree_Node<segment_vector> &tree_vertices) {
    // 首先拿到前三个，
    while (new_segments.size() >= 3) {
        segment_vector a = new_segments.at(new_segments.size() - 3);
        segment_vector b = new_segments.at(new_segments.size() - 2);
        segment_vector c = new_segments.at(new_segments.size() - 1);
        // 判断这三个点是顺时针还是逆时针
        segment_vector ab = b - a;
        segment_vector ac = c - a;
        float area = ab.single_area(ac);
        if (area >= 0 && no_point_in_line_clockwise_direction(a, c, tree_vertices)) {
            // 那么这里是逆时针,并且 所以顶点都不在 ac 的x轴范围内的点，都不在逆时针的方向上
            new_segments.erase(new_segments.begin() + 1);
        } else {
            // 需要将这三个点作为一个三角形进行输出
            segment_vector set_to_last = *new_segments.begin();
            new_segments.erase(new_segments.begin());
            new_segments.push_back(set_to_last);
        }
    }
}

bool ear_clip_algorithm_no_efficient(std::vector<triangle> &result_segments,
                                     std::vector<segment_vector> &new_segments,
                                     std::vector<segment_vector> &tree_vertices) {
    if (new_segments.size() < 3) {
        return false;
    }

    // 首先拿到前三个，
    while (new_segments.size() > 3) {
        segment_vector a = new_segments.at(0);
        segment_vector b = new_segments.at(1);
        segment_vector c = new_segments.at(2);
        // 判断这三个点是顺时针还是逆时针
        segment_vector ab = b - a;
        segment_vector ac = c - a;
        float area = ab.single_area(ac);
        if (area >= 0 && no_point_in_line_clockwise_direction_no_efficient(a, c, b, tree_vertices)) {
            // 那么这里是逆时针,并且 所以顶点都不在 ac 的x轴范围内的点，都不在逆时针的方向上
            new_segments.erase(new_segments.begin() + 1);
            triangle t{{a.x, a.y}, {b.x, b.y}, {c.x, c.y}};
            result_segments.push_back(t);
            // std::cout << "  expect_triangles.push_back(triangle{"
            //         << "{" << a.x << "," << a.y << "},"
            //         << "{" << b.x << "," << b.y << "},"
            //         << "{" << c.x << "," << c.y << "}" << "});"
            //         << std::endl;
        } else {
            // 需要将这三个点作为一个三角形进行输出
            segment_vector set_to_last = *new_segments.begin();
            new_segments.erase(new_segments.begin());
            new_segments.push_back(set_to_last);
        }
    }
    if (new_segments.size() == 3) {
        // 清楚全部的内容，
        // 并进行三角化
        // 得到相应的结果
        triangle t{
            {new_segments.at(0).x, new_segments.at(0).y},
            {new_segments.at(1).x, new_segments.at(1).y},
            {new_segments.at(2).x, new_segments.at(2).y}
        };
        result_segments.push_back(t);
        return true;

        // std::cout << "  expect_triangles.push_back(triangle{"
        //         << "{" << new_segments.at(0).x << "," << new_segments.at(0).y << "},"
        //         << "{" << new_segments.at(1).x << "," << new_segments.at(1).y << "},"
        //         << "{" << new_segments.at(2).x << "," << new_segments.at(2).y << "}" << "});"
        //         << std::endl;
    }

    return false;
}
