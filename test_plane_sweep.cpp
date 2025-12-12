//
// Created by 潘鑫 on 2025/10/6.
//
#include <gtest/gtest.h>

#include "binary_Tree_Node.h"
#include "tree_function.h"
#include "include/base_element/geometry/triangle.h"
#include "half_edge.h"
#include "index_binary_tree.h"
#include "index_binary_tree_node.h"
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
    std::set<int> clean_from_tree;
    std::set<int> re_insert_to_tree; // 那么另一个选择应该是set,可以用set来进行消除重复

    enum intersection_type {
        no_intersect,
        is_intersect,
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
            // 点重复时，相交的事件最大，最后处理
            if (if_intersect == intersection_type::is_intersect &&
                right.if_intersect == intersection_type::no_intersect)
                return false; // 左边大于右边
            if (if_intersect == intersection_type::no_intersect &&
                right.if_intersect == intersection_type::is_intersect)
                return true; //  右边大于左边
            // if_intersect 相等

            if (left_or_right == left_or_right_type::right &&
                right.left_or_right == left_or_right_type::left)
                return false;
            if (left_or_right == left_or_right_type::left &&
                right.left_or_right == left_or_right_type::right)
                return true;
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


void init_all_segments(half_edge_struct<vertex_xy> &hf) {
    hf.create_loop({1, 2}, {2, 5});
    hf.create_loop({1, 2}, {4, 2});
    hf.create_loop({2, 5}, {4, 2});
    hf.create_loop({2, 3}, {7, 5});
    hf.create_loop({6, -1}, {7, 5});
    hf.create_loop({2, 3}, {6, -1});
}

// 上面给出了来的左右点是对的，之后给出的话，两条边，小的不一定是起点。


auto create_event_tree(
    half_edge_struct<vertex_xy> &hf) {
    auto tree = new Index_Binary_Tree<event_point>;
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
                // 新的左端点进入时，必然会出现等于的情况
                std::cout << " intersect point :" << result.x << "  " << result.y << std::endl;
                // 居然没有做测试，只是随便写了两个点，进行手动判断

                event_point temp{};
                temp.x = result.x;
                temp.y = result.y;
                temp.left_or_right = event_point::left;
                temp.if_intersect = event_point::is_intersect;

                temp.incident_half_edge = -1;


                // 找不到就插入，能找到，则不动
                auto data = event_points->tree_find_data(temp);
                if (data != nullptr && data->if_intersect == event_point::is_intersect) {
                    // 并且已经是相交时，才可以进行合并
                    if (ab.end_point == result)
                        data->clean_from_tree.insert(ab_incident_half_edge);
                    else
                        data->re_insert_to_tree.insert(ab_incident_half_edge);
                    if (cd.end_point == result)
                        data->clean_from_tree.insert(cd_incident_half_edge);
                    else
                        data->re_insert_to_tree.insert(cd_incident_half_edge);
                } else {
                    if (ab.end_point == result)
                        temp.clean_from_tree.insert(ab_incident_half_edge);
                    else
                        temp.re_insert_to_tree.insert(ab_incident_half_edge);
                    if (cd.end_point == result)
                        temp.clean_from_tree.insert(cd_incident_half_edge);
                    else
                        temp.re_insert_to_tree.insert(cd_incident_half_edge);
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


std::vector<event_point> get_intersect_point(half_edge_struct<vertex_xy> &hf) {
    auto event_tree = create_event_tree(hf);

    std::vector<event_point> result;
    binary_Tree_Node<ray_2d> *ray_root;
    ray_root = nullptr; // 忽然发现这里插入的时候是有问题的
    v_index mini_node_index;
    for (; mini_node_index = event_tree->minimum(event_tree->get_root_index()),
           event_tree->tree_minimum_data() != nullptr;) {
        // 居然有一个空指针检查在这里，终于的是很像唯一一个
        auto current_for_min = event_tree->tree_minimum_data();
        auto current_half_edge = event_tree->tree_minimum_data()->incident_half_edge;
        auto temp_x = current_for_min->x;
        auto temp_y = current_for_min->y;
        // 上面一行没什么用，只是方便在调试时查看当前在哪里
        if (current_for_min->if_intersect == event_point::is_intersect) {
            auto intersect_vertex = current_for_min;
            if (intersect_vertex->re_insert_to_tree.size() == 0) {
            } else if (intersect_vertex->re_insert_to_tree.size() == 1) {
                // size 为 1 时的专属优化， size 为 2 时也可以添加一个专属优化
                for (auto re_insert_edge: intersect_vertex->re_insert_to_tree) {
                    auto current_half_edge_node = tree_find_value(ray_root, ray_2d::get_ray_2d(hf, re_insert_edge));
                    auto predecessor_half_edge_node = current_half_edge_node->tree_predecessor(current_half_edge_node);
                    auto successor_half_edge_node = current_half_edge_node->tree_successor(current_half_edge_node);
                    test_two_node_if_intersect(predecessor_half_edge_node, current_half_edge_node, hf, event_tree);
                    test_two_node_if_intersect(current_half_edge_node, successor_half_edge_node, hf, event_tree);
                }
            } else if (intersect_vertex->re_insert_to_tree.size() >= 2) {
                std::vector<binary_Tree_Node<ray_2d> *> get_ray_need_sort;
                for (auto re_insert_edge: intersect_vertex->re_insert_to_tree) {
                    auto delete_node = tree_find_value(ray_root, ray_2d::get_ray_2d(hf, re_insert_edge));
                    if (delete_node != nullptr) {
                        //正常情况下不应该有这个判断
                        get_ray_need_sort.push_back(delete_node);
                    }
                }
                // get_ray_need_sort 碰到的一个问题，最后一个交点的时候得到的为零，先这样解决
                // 实际上的是怎么回事还需要研究
                if (get_ray_need_sort.size() >= 2) {
                    std::vector<binary_Tree_Node<ray_2d> *> sort_node;

                    std::set<binary_Tree_Node<ray_2d> *> pre_and_success;
                    // 最前面和最后面的两条线段，不在同一个交点的线段

                    binary_Tree_Node<ray_2d> *min_node = nullptr;
                    for (auto insert_node: get_ray_need_sort) {
                        auto predecessor_half_edge_node = insert_node->tree_predecessor(insert_node);
                        auto successor_half_edge_node = insert_node->tree_successor(insert_node);
                        if (predecessor_half_edge_node != nullptr)
                            pre_and_success.insert(predecessor_half_edge_node);
                        else
                            min_node = insert_node;
                        if (successor_half_edge_node != nullptr)
                            pre_and_success.insert(successor_half_edge_node);
                    }
                    for (auto insert_node: get_ray_need_sort) {
                        pre_and_success.erase(insert_node);
                    }
                    if (min_node == nullptr) {
                        // 下面这一行不对
                        for (auto node: pre_and_success) {
                            for (auto insert_node: get_ray_need_sort) {
                                auto predecessor_half_edge_node = insert_node->tree_predecessor(insert_node);
                                if (predecessor_half_edge_node == node) {
                                    min_node = insert_node;
                                }
                            }
                        }
                    }

                    // 需要一个排序
                    auto x = intersect_vertex->x;
                    std::sort(get_ray_need_sort.begin(), get_ray_need_sort.end(),
                              [x](binary_Tree_Node<ray_2d> *a, binary_Tree_Node<ray_2d> *b) {
                                  return ray_2d::compare(a->data, b->data, x);
                              });

                    assert(min_node != nullptr);
                    // 取值，并按顺序赋值
                    std::vector<ray_2d> ray_2d_vector;
                    for (auto insert_node: get_ray_need_sort) {
                        ray_2d_vector.push_back(insert_node->data);
                    }
                    for (auto temp_ray_2d: ray_2d_vector) {
                        min_node->data = temp_ray_2d;
                        min_node = min_node->tree_successor(min_node);
                    }
                    // 进行比较查看是否存在相交
                    for (auto insert_node: get_ray_need_sort) {
                        for (auto node: pre_and_success) {
                            test_two_node_if_intersect(node, insert_node, hf, event_tree);
                        }
                    }
                }
            }

            result.push_back(*current_for_min);
        } else if (current_for_min->left_or_right == event_point::left_or_right_type::left) {
            ray_root = ray_root->tree_insert_value(ray_root, ray_2d::get_ray_2d(hf, current_half_edge));
            auto current_half_edge_node = tree_find_value(ray_root, ray_2d::get_ray_2d(hf, current_half_edge));
            auto predecessor_half_edge_node = current_half_edge_node->tree_predecessor(current_half_edge_node);
            auto successor_half_edge_node = current_half_edge_node->tree_successor(current_half_edge_node);
            test_two_node_if_intersect(predecessor_half_edge_node, current_half_edge_node, hf, event_tree);
            test_two_node_if_intersect(current_half_edge_node, successor_half_edge_node, hf, event_tree);
        } else if (current_for_min->left_or_right == event_point::left_or_right_type::right) {
            auto delete_node = tree_find_value_with_sweep_line(ray_root, ray_2d::get_ray_2d(hf, current_half_edge),
                                                               current_for_min->x - 0.00001,
                                                               std::bind(&ray_2d::compare, std::placeholders::_1,
                                                                         std::placeholders::_2, std::placeholders::_3));
            if (delete_node == nullptr) {
                std::vector<binary_Tree_Node<ray_2d> *> inorder;
                auto result_vector = inorder_tree_walk_with_stack(ray_root, &inorder);
                for (auto node: *result_vector) {
                    if (node->data == ray_2d::get_ray_2d(hf, current_half_edge)) {
                        delete_node = node;
                        break;
                    }
                }
            } // 一定要删除，通过循环的方式排出找不到的情况
            ray_root = ray_root->delete_node_from_binary_search_tree(ray_root, delete_node);
        }
        event_tree->delete_node(mini_node_index);
    }
    auto ssd = result.size();
    return result;
}


TEST(test_edge, test_create_edge) {
    half_edge_struct<vertex_xy> hf;
    init_all_segments(hf);
    auto result = get_intersect_point(hf);
    int a = 0;
}
