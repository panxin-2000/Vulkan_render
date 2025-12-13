//
// Created by 潘鑫 on 2025/10/6.
//
#include <gtest/gtest.h>

#include "binary_Tree_Node.h"
#include "tree_function.h"
#include "base_element/geometry/triangle.h"
#include "base_element/half_edge/half_edge_struct.h"
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


class event_point {
public:
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

    left_or_right_type left_or_right;

    intersection_type if_intersect; // 暂时没有办法的一个办法了 // 用于判断是否当前端点或者其他是否有在使用

    event_point() {
    }

    event_point(float x, float y, left_or_right_type left_or_right, intersection_type if_intersect) : x(x), y(y),
        left_or_right(left_or_right),
        if_intersect(if_intersect) {
    }


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

template<typename T1>
void add_intersect_event(Segment<Point_2> ab, int ab_incident_half_edge, Segment<Point_2> cd, int cd_incident_half_edge,
                         Point_2 result, T1 event_points) {
    event_point temp{result.x, result.y, event_point::left, event_point::is_intersect};
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
}


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
        Segment<Point_2> current_segment = hf.get_segment(half_edge.twin_half_edge);
        auto temp_l_or_r = event_point::right;
        if (current_segment.start_point.x < current_segment.end_point.x) {
            temp_l_or_r = event_point::left;
        } else if (current_segment.start_point.x == current_segment.end_point.x &&
                   current_segment.start_point.y < current_segment.end_point.y) {
            temp_l_or_r = event_point::left;
        }
        event_point temp{
            current_segment.start_point.x, current_segment.start_point.y, temp_l_or_r, event_point::no_intersect
        };
        temp.incident_half_edge = half_edge.twin_half_edge;
        tree->add_new_node(temp);
    }
    return tree;
}


template<typename T, typename T1>
bool test_two_node_if_intersect(T left_node, T right_node, half_edge_struct<vertex_xy> &hf, T1 event_points) {
    if (left_node != nullptr && right_node != nullptr) {
        return test_two_node_if_intersect(left_node->data, right_node->data, hf, event_points);
    }
    return false;
}

template<typename T1>
bool test_two_node_if_intersect(ray_2d &left_data, ray_2d &right_data, half_edge_struct<vertex_xy> &hf,
                                T1 event_tree) {
    auto temp = hf.get_opposite_edge_index(left_data.incident_half_edge);
    auto temp_2 = hf.get_vertices_index(temp);
    auto temp_3 = hf.get_vertices_index(right_data.incident_half_edge);
    if (temp_2 == temp_3) return false;
    // 上面加的其实是有问题的，因为并没有考虑到四条边相交于同一个点的情况
    // 还有一个问题，应该是默认进入的时候都是靠左的为起点，靠右的为终点，create_event_queue 应该是筛选过的，都是左先进的
    // 但是应该不是很彻底

    // 两条线段判断是否相交
    auto ab_incident_half_edge = left_data.incident_half_edge;
    auto cd_incident_half_edge = right_data.incident_half_edge;
    Segment<Point_2> ab = hf.get_segment(ab_incident_half_edge);
    Segment<Point_2> cd = hf.get_segment(cd_incident_half_edge);

    if (intersect(ab, cd) == true) {
        auto result = ab.get_intersect_result(cd);
        if (event_tree->tree_minimum_data()->x <= result.x) {
            add_intersect_event(ab, ab_incident_half_edge, cd, cd_incident_half_edge, result, event_tree);
            return true;
        }
    }
    return false;
}

// 然后我怎么才能建立这个结构呢？
//  其实应该先写一个最简单暴力的来，不然不太好玩
//
binary_Tree_Node<ray_2d> *get_mini_node(std::vector<binary_Tree_Node<ray_2d> *> get_ray_need_sort) {
    binary_Tree_Node<ray_2d> *min_node = nullptr;
    std::set<binary_Tree_Node<ray_2d> *> pre_and_success;

    for (auto insert_node: get_ray_need_sort) {
        auto predecessor_half_edge_node = binary_Tree_Node<ray_2d>::tree_predecessor(insert_node);
        if (predecessor_half_edge_node != nullptr)
            pre_and_success.insert(predecessor_half_edge_node);
        else
            min_node = insert_node;
    }
    for (auto insert_node: get_ray_need_sort) {
        pre_and_success.erase(insert_node);
    }
    if (pre_and_success.empty() == false) {
        min_node = *pre_and_success.begin();
        min_node = binary_Tree_Node<ray_2d>::tree_successor(min_node);
    }
    return min_node;
}

template<typename T1>
void check_pre_and_success(binary_Tree_Node<ray_2d> *ray_root, half_edge_struct<vertex_xy> &hf, T1 event_tree,
                           int re_insert_edge) {
    auto current_half_edge_node = tree_find_value(ray_root, ray_2d::get_ray_2d(hf, re_insert_edge));
    auto predecessor_half_edge_node = binary_Tree_Node<ray_2d>::tree_predecessor(current_half_edge_node);
    auto successor_half_edge_node = binary_Tree_Node<ray_2d>::tree_successor(current_half_edge_node);
    test_two_node_if_intersect(predecessor_half_edge_node, current_half_edge_node, hf, event_tree);
    test_two_node_if_intersect(current_half_edge_node, successor_half_edge_node, hf, event_tree);
}

template<typename T1>
void check_pre_and_success(Index_Binary_Tree<ray_2d> *ray_root, half_edge_struct<vertex_xy> &hf, T1 event_tree,
                           int re_insert_edge) {
    const auto data = ray_2d::get_ray_2d(hf, re_insert_edge);
    const auto current_index = ray_root->tree_find_index(data);
    auto current_data = ray_root->tree_find_data(data);
    auto predecessor_data = ray_root->predecessor_data(current_index);
    auto successor_data = ray_root->successor_data(current_index);
    test_two_node_if_intersect(predecessor_data, current_data, hf, event_tree); // 这两行应该是编译不过的
    test_two_node_if_intersect(current_data, successor_data, hf, event_tree);
}

std::vector<event_point> get_intersect_point(half_edge_struct<vertex_xy> &hf) {
    auto event_tree = create_event_tree(hf);

    std::vector<event_point> result;
    binary_Tree_Node<ray_2d> *ray_root;
    ray_root = nullptr; // 忽然发现这里插入的时候是有问题的
    for (; event_tree->tree_minimum_data() != nullptr;) {
        // 居然有一个空指针检查在这里，终于的是很像唯一一个
        const auto mini_node_index = event_tree->tree_minimum_index();
        const auto current_loop_min = event_tree->tree_minimum_data();
        const auto current_half_edge = event_tree->tree_minimum_data()->incident_half_edge;
        auto temp_x = current_loop_min->x;
        auto temp_y = current_loop_min->y;
        // 上面一行没什么用，只是方便在调试时查看当前在哪里
        if (current_loop_min->if_intersect == event_point::is_intersect) {
            auto intersect_event = current_loop_min;
            if (intersect_event->re_insert_to_tree.empty() == true) {
                // 有问题，可能少了一个判断
            } else if (intersect_event->re_insert_to_tree.size() == 1) {
                // size 为 1 时的专属优化， size 为 2 时也可以添加一个专属优化
                for (auto re_insert_edge: intersect_event->re_insert_to_tree) {
                    check_pre_and_success(ray_root, hf, event_tree, re_insert_edge);
                }
            } else if (intersect_event->re_insert_to_tree.size() >= 2) {
                std::vector<binary_Tree_Node<ray_2d> *> get_ray_need_sort;
                for (const auto re_insert_edge: intersect_event->re_insert_to_tree) {
                    auto delete_node = tree_find_value(ray_root, ray_2d::get_ray_2d(hf, re_insert_edge));
                    if (delete_node != nullptr) {
                        //正常情况下不应该有这个判断,tree_find_value 可能有问题
                        get_ray_need_sort.push_back(delete_node);
                    }
                }
                std::vector<ray_2d *> ray_2d_s;
                ray_2d_s.reserve(get_ray_need_sort.size());
                for (const auto insert_node: get_ray_need_sort) {
                    ray_2d_s.push_back(&insert_node->data);
                }
                // get_ray_need_sort 碰到的一个问题，最后一个交点的时候得到的为零，先这样解决
                // 实际上的是怎么回事还需要研究
                if (get_ray_need_sort.size() >= 2) {
                    // 最前面和最后面的两条线段，不在同一个交点的线段
                    auto min_node = get_mini_node(get_ray_need_sort);
                    auto temp_min_node = min_node;

                    // 取值，并按顺序赋值
                    auto ray_2d_vector = ray_2d::get_data_vector(ray_2d_s, intersect_event->x);
                    for (auto temp_ray_2d: ray_2d_vector) {
                        temp_min_node->data = temp_ray_2d;
                        temp_min_node = binary_Tree_Node<ray_2d>::tree_successor(temp_min_node);
                    }
                    auto max_node = temp_min_node;
                    auto real_min_node = binary_Tree_Node<ray_2d>::tree_predecessor(min_node);
                    // 进行比较查看是否存在相交
                    for (auto insert_node: get_ray_need_sort) {
                        test_two_node_if_intersect(real_min_node, insert_node, hf, event_tree);
                        test_two_node_if_intersect(insert_node, max_node, hf, event_tree);
                    }
                }
            }

            result.push_back(*current_loop_min);
        } else if (current_loop_min->left_or_right == event_point::left_or_right_type::left) {
            const auto current_data = ray_2d::get_ray_2d(hf, current_half_edge); // 这里又出现了问题导致不能正常运行
            ray_root = ray_root->tree_insert_value(ray_root, current_data);
            check_pre_and_success(ray_root, hf, event_tree, current_half_edge);
        } else if (current_loop_min->left_or_right == event_point::left_or_right_type::right) {
            const auto current_data = ray_2d::get_ray_2d(hf, current_half_edge); // 这里又出现了问题导致不能正常运行
            auto delete_node = tree_find_value_with_sweep_line(ray_root, current_data,
                                                               std::bind(&ray_2d::compare, std::placeholders::_1,
                                                                         std::placeholders::_2,
                                                                         current_loop_min->x - 0.00001));
            assert(delete_node != nullptr); // 一定要删除，通过循环的方式排出找不到的情况
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
