//
// Created by 潘鑫 on 2025/10/6.
//
#include <gtest/gtest.h>

#include "binary_Tree_Node.h"
#include "tree_function.h"
#include "include/base_element/geometry/triangle.h"
#include "half_edge.h"
#include "index_binary_tree.h"
#include "index_tree_node.h"
#include  "segment_end_ponit_and_gradient.h"
// 既然我们这里需要使用plane sweep 算法，
// 那么先说说这个算法是怎么实现的呢？
// 需要首先需要一系列的边，
// 之后才能判断两条边是否相交，
// 然后需要什么呢？需要将全部的边的端点排序好之后添加到一个向量中
// 之后，从这个向量中，不断的拿出端点所对应的边
// 将边添加到树中，每次添加一个边，
// 问题是在树中是按照什么来进行排列的呢？
// 在向量中是按照x轴来排列的，那么到树中的话，是需要按照y轴来排列的
// 左端点是添加到树中，右端点是从树中移出，如果右相同的x值，那么左端点要在右端点之前
//
// 我似乎有一个记录，之前的时候应该手画过这个过程
// 看书的时候发现了一个新的不一样的地方，就是需要判断两条线谁在谁的上方

// 我们可以先这样做，判断右端点离开时是否和其他任何一条线相交，这个是一个最简单的办法
// 因为只是按照x轴的区间进行分类

// 之后需要做更负责的内容，
// 怎么才能判断是否出现了变化呢？
// 新的端点进来的时候需要添加什么样的内容？
// 至少需要先做出一个结果，之后再更改
//


struct event_point {
    float x, y;
    int incident_half_edge;
    int intersect_half_edge_1;
    int intersect_half_edge_2;
    std::set<int> start_points_edge; // 那么另一个选择应该是set,可以用set来进行消除重复
    std::set<int> end_points_edge;
    std::set<int> middle_points_edge;

    enum intersection_type {
        is_intersect,
        no_intersect,
    };

    enum left_or_right_type {
        left,
        right,
    };

    bool left_or_right;

    bool if_intersect; // 暂时没有办法的一个办法了 // 用于判断是否当前端点或者其他是否有在使用
    bool operator<(const event_point &right) const {
        if (x < right.x) {
            // 先比较x轴，x轴小的为小
            return true;
        } else if (x == right.x && y < right.y) {
            // 之后再比较y轴，y轴小的为小
            return true;
        } else if (x == right.x && y == right.y) {
            if (left_or_right == left_or_right_type::right &&
                right.left_or_right == left_or_right_type::left)
                return false;
            if (left_or_right == left_or_right_type::left &&
                right.left_or_right == left_or_right_type::right)
                return true;
            if (left_or_right == left_or_right_type::left &&
                right.left_or_right == left_or_right_type::left) {
                // 同左 , 交点大一点
                if (if_intersect == intersection_type::is_intersect &&
                    right.if_intersect == intersection_type::no_intersect)
                    return false; // 左边大于右边
                if (if_intersect == intersection_type::no_intersect &&
                    right.if_intersect == intersection_type::is_intersect)
                    return true; //  右边大于左边
            }
            if (left_or_right == left_or_right_type::left &&
                right.left_or_right == left_or_right_type::left) {
                // 同右 , 交点小一点
                if (if_intersect == intersection_type::is_intersect &&
                    right.if_intersect == intersection_type::no_intersect)
                    return true; // 左边大于右边
                if (if_intersect == intersection_type::no_intersect &&
                    right.if_intersect == intersection_type::is_intersect)
                    return false; //  右边大于左边
            }
            if (incident_half_edge < right.incident_half_edge) {
                return true; // 左边大于右边
            }
            return false;
        }
        return false;
    }

    bool operator==(const event_point &right) const {
        if (x == right.x && y == right.y &&
            if_intersect == right.if_intersect &&
            left_or_right == right.left_or_right) {
            return true;
        }
        return false;
    }

    bool operator>(const event_point &right) const {
        return !this->operator<(right);
    }
};


template<typename T>
bool if_half_edge_in_tree(binary_Tree_Node<T> *root, T temp) {
    if (tree_find_value(root, temp) == nullptr) {
        return false;
    } else {
        return true;
    }
}


void init_all_segments(half_edge_struct<vertex_xy> &hf) {
    hf.create_loop({1, 1}, {2, 2});
    hf.create_loop({3, 3}, {4, 4});
    hf.create_loop({0, 2}, {2, 0});
    hf.create_loop({0, 6.5}, {6.5, 0});
}

// 上面给出了来的左右点是对的，之后给出的话，两条边，小的不一定是起点。

std::priority_queue<event_point, std::vector<event_point>, std::greater<> > &create_event_queue(
    half_edge_struct<vertex_xy> &hf) {
    auto event_points = new std::priority_queue<event_point, std::vector<event_point>, std::greater<> >;
    for (auto vertice: hf.vertices) {
        event_point temp{};
        temp.x = vertice.x;
        temp.y = vertice.y;
        temp.incident_half_edge = vertice.incident_half_edge;
        temp.intersect_half_edge_1 = -1;
        temp.intersect_half_edge_2 = -1;
        temp.if_intersect = event_point::no_intersect;
        event_points->push(temp);
    }
    return *event_points;
}

auto create_event_tree(
    half_edge_struct<vertex_xy> &hf) {
    auto tree = new index_binary_Tree<event_point, index_Tree_Node<event_point> >;
    for (auto half_edge: hf.half_edges) {
        event_point temp{};
        Segment<Point_2> current_segment = hf.get_segment(half_edge.twin_half_edge);
        temp.x = current_segment.start_point.x;
        temp.y = current_segment.start_point.y;
        temp.incident_half_edge = half_edge.twin_half_edge;
        temp.left_or_right = event_point::right;
        if (current_segment.start_point.x < current_segment.end_point.x) {
            temp.left_or_right = event_point::left;
        } else if (current_segment.start_point.x == current_segment.end_point.x &&
                   current_segment.start_point.y < current_segment.end_point.y) {
            temp.left_or_right = event_point::left;
        }
        temp.intersect_half_edge_1 = -1;
        temp.intersect_half_edge_2 = -1;
        temp.if_intersect = event_point::no_intersect;
        tree->add_new_node(temp);
    }
    return tree;
}


template<typename T, typename T1>
bool test_two_node_if_intersect(T left_node, T right_node, half_edge_struct<vertex_xy> &hf, T1 event_points) {
    if (left_node != nullptr && right_node != nullptr) {
        auto temp = hf.get_opposite_edge_index(left_node->data.incident_half_edge);
        auto temp_2 = hf.get_vertices_index(temp);
        auto temp_3 = hf.get_vertices_index(right_node->data.incident_half_edge);
        if (temp_2 == temp_3) return false;
        // 上面加的其实是有问题的，因为并没有考虑到四条边相交于同一个点的情况
        // 还有一个问题，应该是默认进入的时候都是靠左的为起点，靠右的为终点，create_event_queue 应该是筛选过的，都是左先进的
        // 但是应该不是很彻底

        // 两条线段判断是否相交
        auto ab_incident_half_edge = left_node->data.incident_half_edge;
        auto cd_incident_half_edge = right_node->data.incident_half_edge;
        Segment<Point_2> ab = hf.get_segment(ab_incident_half_edge);
        Segment<Point_2> cd = hf.get_segment(cd_incident_half_edge);

        if (intersect(ab, cd) == true) {
            // 如果相交，把交点插入到事件点中，并且需要判断交点是否在扫描线之后
            auto result = ab.get_intersect_result(cd);
            if (event_points->tree_minimum_data()->x <= result.x) {
                // 上面其实应该是有一个奇怪的问题的，那就是小于还是等于？
                // 问题就是添加一个等于是否会出现循环的问题
                // 端点B 进入，判断和端点A相交
                // 现在扫面线还在端点A，因为A还没有退出， //最重要的，如果没有等于，这里是不会进入的
                // 然后交换AB，检测B是否与之下的线段相交，A是否与之后上的
                // 只要不再判断AB相交就行，
                // 之后是A进行判断的上下的相交判断，扫描线已经在A之后了
                // 不会再进入了
                // 如果有多个点呢？不能在这里考虑，因为会重复交换AB，造成另一个问题
                std::cout << " intersect point :" << result.x << "  " << result.y << std::endl;
                // 居然没有做测试，只是随便写了两个点，进行手动判断

                event_point temp{};
                temp.x = result.x;
                temp.y = result.y;
                temp.left_or_right = event_point::left;
                temp.if_intersect = event_point::is_intersect;

                temp.incident_half_edge = -1;
                temp.intersect_half_edge_1 = ab_incident_half_edge;
                temp.intersect_half_edge_2 = cd_incident_half_edge;


                // 找不到就插入，能找到，则不动
                auto data = event_points->tree_find_data(temp);
                if (data != nullptr && data->if_intersect == event_point::is_intersect) {
                    // 并且已经是相交时，才可以进行合并
                    if (ab.start_point == result)
                        data->start_points_edge.insert(ab_incident_half_edge);
                    else if (ab.end_point == result)
                        data->end_points_edge.insert(ab_incident_half_edge);
                    else
                        data->middle_points_edge.insert(ab_incident_half_edge);
                    if (cd.start_point == result)
                        data->start_points_edge.insert(cd_incident_half_edge);
                    else if (cd.end_point == result)
                        data->end_points_edge.insert(cd_incident_half_edge);
                    else
                        data->middle_points_edge.insert(cd_incident_half_edge);
                } else {
                    if (ab.start_point == result)
                        temp.start_points_edge.insert(ab_incident_half_edge);
                    else if (ab.end_point == result)
                        temp.end_points_edge.insert(ab_incident_half_edge);
                    else
                        temp.middle_points_edge.insert(ab_incident_half_edge);
                    if (cd.start_point == result)
                        temp.start_points_edge.insert(cd_incident_half_edge);
                    else if (cd.end_point == result)
                        temp.end_points_edge.insert(cd_incident_half_edge);
                    else
                        temp.middle_points_edge.insert(cd_incident_half_edge);
                    event_points->add_new_node(temp); // 这里只是为了在树中交换，// 交换应该在删除只前
                    // 还需要把相交的点插入一个vector中，用于之后的输出
                }
                return true;
            }
        }
    }
    return false;
}

// 然后我怎么才能建立这个结构呢？
//  其实应该先写一个最简单暴力的来，不然不太好玩
//
TEST(test_edge, test_create_edge) {
    half_edge_struct<vertex_xy> hf;
    init_all_segments(hf);

    auto event_tree = create_event_tree(hf);

    std::vector<event_point> result;

    binary_Tree_Node<ray_2d> *ray_root;
    ray_root = nullptr; // 忽然发现这里插入的时候是有问题的
    v_index mini_node_index;
    for (; mini_node_index = event_tree->minimum(event_tree->get_root_index()),
           event_tree->tree_minimum_data() != nullptr;) {
        // 居然有一个空指针检查在这里，终于的是很像唯一一个
        auto temo = event_tree->tree_minimum_data();
        auto current_half_edge = event_tree->tree_minimum_data()->incident_half_edge;
        vertex_xy current_vertex{
            event_tree->tree_minimum_data()->x, event_tree->tree_minimum_data()->y, current_half_edge
        };
        // 上面一行没什么用，只是方便在调试时查看当前在哪里
        if (event_tree->tree_minimum_data()->if_intersect == event_point::is_intersect) {
            // 是线段中的交点,之后应该如何处理呢？
            // 问题是这应该携带什么信息？需要拿到是那两条边相交的，
            // 之后应该如何处理呢？//交换,既然是相交的，那么他们之前一定是相邻的，交互两个结点就好
            auto intersect_vertex = event_tree->tree_minimum_data();
            auto edge_1_vertex = ray_2d::get_ray_2d(hf, intersect_vertex->intersect_half_edge_1);
            auto edge_2_vertex = ray_2d::get_ray_2d(hf, intersect_vertex->intersect_half_edge_2);
            // 因为是auto 所以上面的名字是不对的，但是还是能够继续工作，因为拿到的类型和将要输入的类型是一致的
            auto edge_1_vertex_node = tree_find_value(ray_root, edge_1_vertex);
            auto edge_2_vertex_node = tree_find_value(ray_root, edge_2_vertex);
            std::swap(edge_1_vertex_node->data, edge_2_vertex_node->data);
            // 能判断相交的一定是前后的， 1 是前，2 是后的
            auto predecessor_edge_node = edge_1_vertex_node->tree_predecessor(edge_1_vertex_node);
            auto successor_edge_node = edge_2_vertex_node->tree_successor(edge_2_vertex_node);
            test_two_node_if_intersect(predecessor_edge_node, edge_1_vertex_node, hf, event_tree);
            test_two_node_if_intersect(edge_2_vertex_node, successor_edge_node, hf, event_tree);
            // 这里有一个前提条件，就是到底交点的扫描线时，已经全部全部处理完成了
            // 最大的问题是排序,先将左边的全部处理掉，再将交点处理，最后将右边的全部删除
            result.push_back(*event_tree->tree_minimum_data());
        } else if (if_half_edge_in_tree(ray_root, ray_2d::get_ray_2d(hf, current_half_edge)) == false) {
            ray_root = ray_root->tree_insert_value(ray_root, ray_2d::get_ray_2d(hf, current_half_edge));
            auto current_half_edge_node = tree_find_value(ray_root, ray_2d::get_ray_2d(hf, current_half_edge));
            auto predecessor_half_edge_node = current_half_edge_node->tree_predecessor(current_half_edge_node);
            auto successor_half_edge_node = current_half_edge_node->tree_successor(current_half_edge_node);
            test_two_node_if_intersect(predecessor_half_edge_node, current_half_edge_node, hf, event_tree);
            test_two_node_if_intersect(current_half_edge_node, successor_half_edge_node, hf, event_tree);
        } else {
            auto delate_node = tree_find_value(ray_root, ray_2d::get_ray_2d(hf, current_half_edge));
            ray_root = ray_root->delete_node_from_binary_search_tree(ray_root, delate_node);
        }
        event_tree->delete_node(mini_node_index);
    }
}

// 说一下上面的代码有哪些没有完成
// 第一个没有完成的是，多个线段相交在同一位置时应该如何处理
// 第二个其实是水平的线段，在我的代码中应该是垂直线段，斜率为无穷

// 把把 queue 变成一个 tree 吗？ 为什么需要？ 一个需要排序的办法，插入时能够自定义

// 忽然想清楚了，为什么两个树不能合并的原因了，事件点是线段的两端
// 而另一棵树的排序只是和射线相关的内容
