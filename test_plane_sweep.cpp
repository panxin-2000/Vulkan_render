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


template<typename T>
bool if_half_edge_in_tree(binary_Tree_Node<T> *root, T temp) {
    if (tree_find_value(root, temp) == nullptr) {
        return false;
    } else {
        return true;
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

std::priority_queue<event_point, std::vector<event_point>, std::greater<> > &create_event_queue(
    half_edge_struct<vertex_xy> &hf) {
    auto event_points = new std::priority_queue<event_point, std::vector<event_point>, std::greater<> >;
    for (auto vertice: hf.vertices) {
        event_point temp{};
        temp.x = vertice.x;
        temp.y = vertice.y;
        temp.incident_half_edge = vertice.incident_half_edge;
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
            // 右端点是需要删除的，删除之前判断，上下是否都在集合当中，是的话，删除，不需要重新检测，否则需要重新检测
            // for (auto end_edge: intersect_vertex->clean_from_tree) {
            //     auto delate_node = tree_find_value(ray_root, ray_2d::get_ray_2d(hf, end_edge));
            //     ray_root = ray_root->delete_node_from_binary_search_tree(ray_root, delate_node);
            // }
            // 删除没有问题，重新插入有问题，不能在这里删除

            if (intersect_vertex->re_insert_to_tree.size() == 0) {
            } else if (intersect_vertex->re_insert_to_tree.size() == 1) {
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
                auto x = intersect_vertex->x;
                // 需要一个排序
                std::sort(get_ray_need_sort.begin(), get_ray_need_sort.end(),
                          [x](binary_Tree_Node<ray_2d> *a, binary_Tree_Node<ray_2d> *b) {
                              const ray_2d &right = b->data;
                              const ray_2d &left = a->data;
                              // float current_segment_y = left.y + left.gradient * (x - left.x);
                              // float right_segment_y = right.y + right.gradient * (x - right.x);
                              float current_segment_y = left.y + left.gradient * (right.compare_x_position - x);
                              float right_segment_y = right.y + right.gradient * (right.compare_x_position - right.x);

                              if (abs(current_segment_y - right_segment_y) < 0.00001) {
                                  if (left.gradient < right.gradient) {
                                      return true; // 梯度的比较
                                  }
                                  return false;
                              }
                              if (current_segment_y < right_segment_y) {
                                  return true;
                              }
                              return false;
                          });
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
                assert(min_node != nullptr);


                for (auto insert_node: get_ray_need_sort) {
                    std::swap(min_node->data, insert_node->data);
                    min_node = insert_node->tree_successor(insert_node);
                }
                for (auto insert_node: get_ray_need_sort) {
                    for (auto node: pre_and_success) {
                        test_two_node_if_intersect(node, insert_node, hf, event_tree);
                    }
                }


                // 遍历一遍，找到现在的顺序？ 不需要吧，随便找一个点，向前并向后，直到不在set中，就必然不是了
                // 保存一下这个顺序，目前是已经知道相交点了，ray 也是能够得出的，应该是有排序的办法的
                // 排序完之后呢？拿到之外的最下与最上，与每个都进行比较,也可以直到不相交时就停止上或者下
                // 结束
                // 一个比较奇怪的想法是，能够删除之后，再重新插入？次数会爆炸，不要紧，重点是确定进行比较的两个对象
                // 重新插入时会调用光线的排序，红黑树，在这个的效率上必然不算高
                // 这里有一个前提条件，就是到底交点的扫描线时，已经全部全部处理完成了
                // 最大的问题是排序,先将左边的全部处理掉，再将交点处理，最后将右边的全部删除
            }

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
    auto ssd = result.size();
    return result;
}

// 说一下上面的代码有哪些没有完成
// 第一个没有完成的是，多个线段相交在同一位置时应该如何处理
// 第二个其实是水平的线段，在我的代码中应该是垂直线段，斜率为无穷

// 把把 queue 变成一个 tree 吗？ 为什么需要？ 一个需要排序的办法，插入时能够自定义

// 忽然想清楚了，为什么两个树不能合并的原因了，事件点是线段的两端
// 而另一棵树的排序只是和射线相关的内容

// 忽然想到来另一个问题，那就是线段之间不可以重合

TEST(test_edge, test_create_edge) {
    half_edge_struct<vertex_xy> hf;
    init_all_segments(hf);
    auto result = get_intersect_point(hf);
    int a = 0;
}
