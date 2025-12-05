//
// Created by 潘鑫 on 2025/11/1.
//

#ifndef TRAPEZOID_H
#define TRAPEZOID_H
#include "base_element/AABB_bounding_box.h"

// 我希望四边形有什么样的结构呢？
// 四边形肯定是需要有四个顶点的
// 以及四条边，只有四条边就够了吗
// 还需要知道这个四边形属于那个face
// 怎么才能知道属于那个face呢？
// 创建与有向无环图的双向连接
// 或者说，这个结构需要添加到树中
//
struct TRAPEZOID {
    Point_2 left_upper;
    Point_2 right_upper;
    Point_2 left_lower;
    Point_2 right_lower;
};


template<class T>
class trapezoid_graph_Node : public Tree_Node<trapezoid_graph_Node<T> > {
    union my_union {
        TRAPEZOID trapezoid;
        Point_2 segment_point;
        Segment<Point_2> segment;

        my_union() {
        }

        ~my_union() {
        }
    };

    my_union trapezoid_union_data;

    enum class graph_enum {
        leaf_node,
        point_node,
        segment_node,
    };

    enum class point_enum {
        left_point,
        right_point,
    };

    graph_enum trapezoid_type;

    // 这里有一个使用模版不断套娃的小技巧
public:
    trapezoid_graph_Node() {
    }

    ~trapezoid_graph_Node() {
    }

    using trapezoid_ptr = trapezoid_graph_Node<T> *;


    // 写完之后，发现其实查找并不是问题，问题是怎么构造的问题
    // 构造的时候最简单的比较好做，然后就是难的部分应该怎么做
    // false left_point     right right_point
    static trapezoid_ptr find_point_in_trapezoid_graph(trapezoid_ptr root_node,
                                                       Point_2 find_point,
                                                       point_enum left_or_right_point =
                                                               point_enum::left_point,
                                                       bool have_help_point = false,
                                                       Point_2 help_point = Point_2(0, 0)) {
        // 既然这里已经看到了查找相关的内容
        // 那么其实需要先构造一个具体的结构
        // 测试一下我这里查找的结果是否是我需要的
        // 既然要做这么一个测试，其实还是只能边做，边更新了
        switch (root_node->trapezoid_type) {
            case graph_enum::leaf_node: {
                // 需要判断是否在梯形内
                auto A_point = root_node->trapezoid_union_data.trapezoid.left_upper;
                auto B_point = root_node->trapezoid_union_data.trapezoid.right_upper;
                auto C_point = root_node->trapezoid_union_data.trapezoid.left_lower;
                auto D_point = root_node->trapezoid_union_data.trapezoid.right_lower;
                if (Point_2::is_anticlockwise(C_point, D_point, find_point) != Point_2::anticlockwise::clockwise &&
                    Point_2::is_anticlockwise(D_point, B_point, find_point) != Point_2::anticlockwise::clockwise &&
                    Point_2::is_anticlockwise(B_point, A_point, find_point) != Point_2::anticlockwise::clockwise &&
                    Point_2::is_anticlockwise(A_point, C_point, find_point) != Point_2::anticlockwise::clockwise) {
                    return root_node;
                } else {
                    return nullptr;
                }
            }
            case graph_enum::point_node: {
                // 需要判断需要查找的点是在当前点点左边还是右边？
                // todo:还需要判断边界条件
                if (find_point.x == root_node->trapezoid_union_data.segment_point.x &&
                    left_or_right_point == point_enum::right_point) {
                    return find_point_in_trapezoid_graph(root_node->left, find_point, left_or_right_point,
                                                         have_help_point, help_point);
                }
                if (find_point.x >= root_node->trapezoid_union_data.segment_point.x) {
                    return find_point_in_trapezoid_graph(root_node->right, find_point, left_or_right_point,
                                                         have_help_point, help_point);
                }
                return find_point_in_trapezoid_graph(root_node->left, find_point, left_or_right_point, have_help_point,
                                                     help_point);
            }
            case graph_enum::segment_node: {
                // 第一个问题是这个点有多个线段应该怎么办？

                Point_2 a = root_node->trapezoid_union_data.segment.start_point;
                Point_2 b = root_node->trapezoid_union_data.segment.end_point;
                Point_2 c = find_point;
                // 这里还需要添加判断，
                float temp = Point_2::single_area(a, b, c);
                if (abs(temp) < 0.0000001) {
                    float temp2 = Point_2::single_area(a, b, help_point);
                    if (temp2 > 0.0000001) {
                        return find_point_in_trapezoid_graph(root_node->left, find_point, left_or_right_point,
                                                             have_help_point, help_point);
                    } else {
                        return find_point_in_trapezoid_graph(root_node->right, find_point, left_or_right_point,
                                                             have_help_point, help_point);
                    }
                } else if (temp > 0.0000001) {
                    return find_point_in_trapezoid_graph(root_node->left, find_point, left_or_right_point,
                                                         have_help_point, help_point);
                } else {
                    return find_point_in_trapezoid_graph(root_node->right, find_point, left_or_right_point,
                                                         have_help_point, help_point);
                }
            }
            default:
                return nullptr;
        }
    }

    static trapezoid_ptr find_right_trapezoid(trapezoid_ptr root_node, trapezoid_ptr trapezoid,
                                              Segment<Point_2> insert_segment) {
        auto left_point = insert_segment.start_point;
        auto right_point = insert_segment.end_point;
        auto E_point = left_point;
        auto F_point = right_point;
        auto D_point = trapezoid->trapezoid_union_data.trapezoid.right_lower;

        auto J_point = Segment<Point_2>::get_segment_point_on_x(E_point, F_point, D_point.x);

        return find_point_in_trapezoid_graph(root_node, J_point,
                                             point_enum::left_point, true,
                                             insert_segment.end_point);
    }

    static trapezoid_ptr add_a_segment(trapezoid_ptr root_node, Segment<Point_2> insert_segment) {
        auto start_point_trapezoid = find_point_in_trapezoid_graph(root_node, insert_segment.start_point,
                                                                   point_enum::left_point, true,
                                                                   insert_segment.end_point);
        auto end_point_trapezoid = find_point_in_trapezoid_graph(root_node, insert_segment.end_point,
                                                                 point_enum::right_point, true,
                                                                 insert_segment.start_point);
        if ((start_point_trapezoid != nullptr) && (end_point_trapezoid != nullptr) &&
            (start_point_trapezoid == end_point_trapezoid)) {
            // 判断是否在一个梯形中，之后 end_point_trapezoid 其实就不用再使用了
            auto new_graph_node = replace_node_in_one_trapezoid(start_point_trapezoid, insert_segment);
            // 将原本包含 start_point_trapezoid 的子树替换为 new_graph_node 的子树
            trapezoid_graph_Node::replace_sub_tree(start_point_trapezoid, new_graph_node);
            // 另一个地方也是有下面这样一行的代码的
            if (root_node == start_point_trapezoid) {
                return new_graph_node;
            }
            return root_node;
            // 第一次添加的时候是可以将根结点直接返回的，之后的时候就不能了，第一种是指针不变，指针里面的内容变掉，第二种是指针需要被更改
            // 上面不需要合并的代码
        } else if (start_point_trapezoid != nullptr && end_point_trapezoid != nullptr) {
            // 第一个问题是左端点重合的问题，需要看右端点
            // 确定是在那个梯形中，
            // 右端点其实也会有重合的，右端点重合有几种不同的情况，首先是只与右侧的线段重合
            // 之后是重合的时候是有左右两侧的线段的

            auto new_graph_node = replace_node_in_multi_trapezoid_left_in_right_out(
                start_point_trapezoid, insert_segment);
            trapezoid_graph_Node::replace_sub_tree(start_point_trapezoid, new_graph_node);
            auto result = new std::vector<trapezoid_ptr>();
            find_all_leaf_node(new_graph_node, *result);
            auto right_trapezoid = find_right_trapezoid(root_node, start_point_trapezoid, insert_segment);
            // 现在的代码其实写的并没有什么意识到应该这么写，但是写下去之后，发现这么写好像刚刚好
            while (right_trapezoid != end_point_trapezoid) {
                // 执行操作，
                auto intern_graph_node = replace_node_in_multi_trapezoid_left_out_right_out(
                    right_trapezoid, insert_segment);
                trapezoid_graph_Node::replace_sub_tree(right_trapezoid, intern_graph_node);
                right_trapezoid = find_right_trapezoid(root_node, right_trapezoid, insert_segment);
            }
            // 相等的时候，执行另一个操作
            auto last_graph_node = replace_node_in_multi_trapezoid_left_out_right_in(right_trapezoid, insert_segment);
            trapezoid_graph_Node::replace_sub_tree(end_point_trapezoid, last_graph_node);

            // 但是这里是需要合并的代码的
            // 将所有的叶子结点添加到队列中
            // 梯形需要合并，来减少梯形结点的数量。


            return root_node;
        }
        return nullptr;
    }


    static trapezoid_ptr replace_node_in_one_trapezoid(
        trapezoid_ptr root, Segment<Point_2> insert_segment) {
        // 最后还是需要返回的，因为根结点可能是会被改变的
        auto left_point = insert_segment.start_point;
        auto right_point = insert_segment.end_point;
        auto A_point = root->trapezoid_union_data.trapezoid.left_upper;
        auto B_point = root->trapezoid_union_data.trapezoid.right_upper;
        auto C_point = root->trapezoid_union_data.trapezoid.left_lower;
        auto D_point = root->trapezoid_union_data.trapezoid.right_lower;
        auto E_point = left_point;
        auto F_point = right_point;
        if (A_point.x != E_point.x && B_point.x != F_point.x) {
            // 找到左右的点，需要判断左右的点，其实都在当前区域内，这个由前置条件完成判断
            // 原本的root 被删除并释放内存  之后 被替换为了                           E_node
            // 因为是梯形，所以需要几个新的点                                       /      \
            //     A---------G--------------H---------B             R_trapezoid        F_node
            //     |         |      S       |         |                               /       \
            //     |         |              |         |                    EF_segment_node      U_trapezoid
            //     |   R     E--------------F    U    |                   /         \
            //     |         |              |         |                  /           \
            //     |         |      T       |         |          S_trapezoid         T_trapezoid
            //     C---------I--------------J---------D
            // 我这里给出了来的其实更加偏向于长方形不过用来做点点示意还是可以的
            // 这几个点还需要计算,计算完成了，开始想怎么构造第一个结构了
            auto G_point = Segment<Point_2>::get_segment_point_on_x(A_point, B_point, E_point.x);
            auto H_point = Segment<Point_2>::get_segment_point_on_x(A_point, B_point, F_point.x);
            auto I_point = Segment<Point_2>::get_segment_point_on_x(C_point, D_point, E_point.x);
            auto J_point = Segment<Point_2>::get_segment_point_on_x(C_point, D_point, H_point.x);
            auto R_trapezoid = init_four_points(A_point, G_point, C_point, I_point);
            auto S_trapezoid = init_four_points(G_point, H_point, E_point, F_point);
            auto T_trapezoid = init_four_points(E_point, F_point, I_point, J_point);
            auto U_trapezoid = init_four_points(H_point, B_point, J_point, D_point);
            // 梯形插入完成了，之后需要做什么呢？
            // 构造结构了，如果将原本的梯形替换为新的内容
            // 需要参考书上的图6-7 来构造相应的结构
            // 第一个不需要一个有E点的结构，第二个，需要一个有F点的结构
            auto E_node = init_points_node(E_point);
            auto F_node = init_points_node(F_point);
            auto EF_segment_node = init_segment_node(E_point, F_point);
            trapezoid_graph_Node::replace_sub_tree_left(E_node, R_trapezoid);
            trapezoid_graph_Node::replace_sub_tree_right(E_node, F_node);
            trapezoid_graph_Node::replace_sub_tree_left(F_node, EF_segment_node);
            trapezoid_graph_Node::replace_sub_tree_right(F_node, U_trapezoid);
            trapezoid_graph_Node::replace_sub_tree_left(EF_segment_node, S_trapezoid);
            trapezoid_graph_Node::replace_sub_tree_right(EF_segment_node, T_trapezoid);
            return E_node;
        } else if (A_point.x == E_point.x && B_point.x == F_point.x) {
            auto EF_segment_node = init_segment_node(E_point, F_point);
            auto S_trapezoid = init_four_points(A_point, B_point, E_point, F_point);
            auto T_trapezoid = init_four_points(E_point, F_point, C_point, D_point);
            trapezoid_graph_Node::replace_sub_tree_left(EF_segment_node, S_trapezoid);
            trapezoid_graph_Node::replace_sub_tree_right(EF_segment_node, T_trapezoid);
            return EF_segment_node;
        } else if (A_point == E_point || C_point == E_point) {
            // 这其实是一种退化的 (degenerate) 情况
            // 找到左右的点，需要判断左右的点，其实都在当前区域内，这个由前置条件完成判断
            // 原本的root 被删除并释放内存  之后 被替换为了
            // 因为是梯形，所以需要几个新的点( AHB 和 GJB 是一条直线)
            //                              H---------B                                 F_node
            //                        *     |         |                               /       \
            //                   *          |         |                              /         \
            //              *        S      |         |                    EF_segment_node      U_trapezoid
            //         *                    |         |                    /       \
            //  A C E-----------------------F    U    |                   /         \
            //         *                    |         |                  /           \
            //              *        T      |         |                 /             \
            //                    *         |         |          S_trapezoid         T_trapezoid
            //                        *     |         |
            //                              J---------D
            //  A---------------------------H---------B                                H---------B
            //  |                           |         |                          *     |         |
            //  |                           |         |                     *          |         |
            //  |                    S      |         |                *        S      |         |
            //  |                           |         |           *                    |         |
            //  C E-------------------------F    U    |    A  E------------------------F    U    |
            //         *                    |         |    |                           |         |
            //              *        T      |         |    |                    T      |         |
            //                    *         |         |    |                           |         |
            //                        *     |         |    |                           |         |
            //                              J---------D    C---------------------------J---------D
            //
            // 我这里给出了来的其实更加偏向于长方形不过用来做点点示意还是可以的
            auto H_point = Segment<Point_2>::get_segment_point_on_x(A_point, B_point, F_point.x);
            auto J_point = Segment<Point_2>::get_segment_point_on_x(C_point, D_point, H_point.x);
            auto S_trapezoid = init_four_points(A_point, H_point, E_point, F_point);
            auto T_trapezoid = init_four_points(E_point, F_point, C_point, J_point);
            auto U_trapezoid = init_four_points(H_point, B_point, J_point, D_point);
            // 梯形插入完成了，之后需要做什么呢？
            // 构造结构了，如果将原本的梯形替换为新的内容
            // 需要参考书上的图6-7 来构造相应的结构
            // 第一个不需要一个有E点的结构，第二个，需要一个有F点的结构
            auto F_node = init_points_node(F_point);
            auto EF_segment_node = init_segment_node(E_point, F_point);
            trapezoid_graph_Node::replace_sub_tree_left(F_node, EF_segment_node);
            trapezoid_graph_Node::replace_sub_tree_right(F_node, U_trapezoid);
            trapezoid_graph_Node::replace_sub_tree_left(EF_segment_node, S_trapezoid);
            trapezoid_graph_Node::replace_sub_tree_right(EF_segment_node, T_trapezoid);
            return F_node;
        } else if (B_point == F_point || D_point == F_point) {
            // 被删除并释放内存 之后 被替换为了                                      E_node
            // 因为是梯形，所以需要几个新的点                                       /      \
            //     A---------G                                      R_trapezoid        \
            //     |         |     *                                                   \
            //     |         |           *                                             \
            //     |         |      S         *                                        \
            //     |         |                    *                                 EF_segment_node
            //     |   R     E------------------------B D F(任意两个点重合)          /         \
            //     |         |                    *                               /           \
            //     |         |      T        *                                   /             \
            //     |         |           *                                      /               \
            //     |         |      *                                     S_trapezoid         T_trapezoid
            //     C---------I
            auto G_point = Segment<Point_2>::get_segment_point_on_x(A_point, B_point, E_point.x);
            auto I_point = Segment<Point_2>::get_segment_point_on_x(C_point, D_point, E_point.x);
            auto R_trapezoid = init_four_points(A_point, G_point, C_point, I_point);
            auto S_trapezoid = init_four_points(G_point, B_point, E_point, F_point);
            auto T_trapezoid = init_four_points(E_point, F_point, I_point, D_point);
            // 梯形插入完成了，之后需要做什么呢？
            // 构造结构了，如果将原本的梯形替换为新的内容
            // 需要参考书上的图6-7 来构造相应的结构
            // 第一个不需要一个有E点的结构，第二个，需要一个有F点的结构
            auto E_node = init_points_node(E_point);
            auto EF_segment_node = init_segment_node(E_point, F_point);
            trapezoid_graph_Node::replace_sub_tree_left(E_node, R_trapezoid);
            trapezoid_graph_Node::replace_sub_tree_right(E_node, EF_segment_node);
            trapezoid_graph_Node::replace_sub_tree_left(EF_segment_node, S_trapezoid);
            trapezoid_graph_Node::replace_sub_tree_right(EF_segment_node, T_trapezoid);

            return E_node;
        }
        return nullptr;
    }

    //
    static trapezoid_ptr replace_node_in_multi_trapezoid_left_in_right_out(
        trapezoid_ptr root, Segment<Point_2> insert_segment) {
        // 最后还是需要返回的，因为根结点可能是会被改变的
        auto left_point = insert_segment.start_point;
        auto right_point = insert_segment.end_point;
        auto A_point = root->trapezoid_union_data.trapezoid.left_upper;
        auto B_point = root->trapezoid_union_data.trapezoid.right_upper;
        auto C_point = root->trapezoid_union_data.trapezoid.left_lower;
        auto D_point = root->trapezoid_union_data.trapezoid.right_lower;
        auto E_point = left_point;
        auto F_point = right_point;
        if (A_point == E_point || C_point == E_point) {
            auto J_point = Segment<Point_2>::get_segment_point_on_x(E_point, F_point, D_point.x);
            auto S_trapezoid = init_four_points(A_point, B_point, E_point, J_point);
            auto T_trapezoid = init_four_points(E_point, J_point, C_point, D_point);
            auto EF_segment_node = init_segment_node(E_point, F_point);
            trapezoid_graph_Node::replace_sub_tree_left(EF_segment_node, S_trapezoid);
            trapezoid_graph_Node::replace_sub_tree_right(EF_segment_node, T_trapezoid);

            return EF_segment_node;
        }
        // 找到左右的点，需要判断左右的点，其实都在当前区域内，这个由前置条件完成判断
        // 原本的root 被删除并释放内存  之后 被替换为了                           E_node
        // 因为是梯形，所以需要几个新的点                                       /      \
        //     A---------G------------------------B             R_trapezoid        \
        //     |         |      S                 |                                 \
        //     |         |                        |                              EF_segment_node
        //     |   R     E------------------------J-----F                       /         \
        //     |         |                        |                            /           \
        //     |         |      T                 |                    S_trapezoid         T_trapezoid
        //     C---------I------------------------D
        // 我这里给出了来的其实更加偏向于长方形不过用来做点点示意还是可以的
        // 这几个点还需要计算,计算完成了，开始想怎么构造第一个结构了
        auto G_point = Segment<Point_2>::get_segment_point_on_x(A_point, B_point, E_point.x);
        auto I_point = Segment<Point_2>::get_segment_point_on_x(C_point, D_point, E_point.x);
        auto J_point = Segment<Point_2>::get_segment_point_on_x(E_point, F_point, D_point.x);
        auto R_trapezoid = init_four_points(A_point, G_point, C_point, I_point);
        auto S_trapezoid = init_four_points(G_point, B_point, E_point, J_point);
        auto T_trapezoid = init_four_points(E_point, J_point, I_point, D_point);
        // 梯形插入完成了，之后需要做什么呢？
        // 构造结构了，如果将原本的梯形替换为新的内容
        // 需要参考书上的图6-7 来构造相应的结构
        // 第一个不需要一个有E点的结构，第二个，需要一个有F点的结构
        auto E_node = init_points_node(E_point);
        auto EF_segment_node = init_segment_node(E_point, F_point);
        trapezoid_graph_Node::replace_sub_tree_left(E_node, R_trapezoid);
        trapezoid_graph_Node::replace_sub_tree_right(E_node, EF_segment_node);
        trapezoid_graph_Node::replace_sub_tree_left(EF_segment_node, S_trapezoid);
        trapezoid_graph_Node::replace_sub_tree_right(EF_segment_node, T_trapezoid);

        return E_node;
    }

    static trapezoid_ptr replace_node_in_multi_trapezoid_left_out_right_in(
        trapezoid_ptr root, Segment<Point_2> insert_segment) {
        // 最后还是需要返回的，因为根结点可能是会被改变的
        auto left_point = insert_segment.start_point;
        auto right_point = insert_segment.end_point;
        auto A_point = root->trapezoid_union_data.trapezoid.left_upper;
        auto B_point = root->trapezoid_union_data.trapezoid.right_upper;
        auto C_point = root->trapezoid_union_data.trapezoid.left_lower;
        auto D_point = root->trapezoid_union_data.trapezoid.right_lower;
        auto E_point = left_point;
        auto F_point = right_point;
        if (B_point == F_point || D_point == F_point) {
            auto J_point = Segment<Point_2>::get_segment_point_on_x(E_point, F_point, C_point.x);
            auto S_trapezoid = init_four_points(A_point, B_point, J_point, F_point);
            auto T_trapezoid = init_four_points(J_point, F_point, C_point, D_point);
            auto EF_segment_node = init_segment_node(E_point, F_point);
            trapezoid_graph_Node::replace_sub_tree_left(EF_segment_node, S_trapezoid);
            trapezoid_graph_Node::replace_sub_tree_right(EF_segment_node, T_trapezoid);
            return EF_segment_node;
        }
        // 找到左右的点，需要判断左右的点，其实都在当前区域内，这个由前置条件完成判断
        // 原本的root 被删除并释放内存  之后 被替换为了                                  F_node
        // 因为是梯形，所以需要几个新的点                                              /      \
        //            A---------G------------------------B                       /        \
        //            |    S    |                        |                      /         R_trapezoid
        //            |         |                        |              EF_segment_node
        //    E-------J---------F             R          |             /         \
        //            |         |                        |            /           \
        //            |    T    |                        |    S_trapezoid         T_trapezoid
        //            C---------I------------------------D
        // 有退化的情况没有考虑，比如AC是同一个点，四边形退化为三角形的情况
        // 更特殊的一点是EF与CD相交或者 EF与AB相交，但是最开始的规定中不允许出现这种情况
        auto G_point = Segment<Point_2>::get_segment_point_on_x(A_point, B_point, F_point.x);
        auto I_point = Segment<Point_2>::get_segment_point_on_x(C_point, D_point, F_point.x);
        auto J_point = Segment<Point_2>::get_segment_point_on_x(E_point, F_point, C_point.x);
        auto R_trapezoid = init_four_points(G_point, B_point, I_point, D_point);
        auto S_trapezoid = init_four_points(A_point, G_point, J_point, F_point);
        auto T_trapezoid = init_four_points(J_point, F_point, C_point, I_point);
        // 梯形插入完成了，之后需要做什么呢？
        // 构造结构了，如果将原本的梯形替换为新的内容
        // 需要参考书上的图6-7 来构造相应的结构
        // 第一个不需要一个有E点的结构，第二个，需要一个有F点的结构
        auto F_node = init_points_node(F_point);
        auto EF_segment_node = init_segment_node(E_point, F_point);
        trapezoid_graph_Node::replace_sub_tree_left(F_node, EF_segment_node);
        trapezoid_graph_Node::replace_sub_tree_right(F_node, R_trapezoid);
        trapezoid_graph_Node::replace_sub_tree_left(EF_segment_node, S_trapezoid);
        trapezoid_graph_Node::replace_sub_tree_right(EF_segment_node, T_trapezoid);

        return F_node;
    }

    static trapezoid_ptr replace_node_in_multi_trapezoid_left_out_right_out(
        trapezoid_ptr root, Segment<Point_2> insert_segment) {
        auto left_point = insert_segment.start_point;
        auto right_point = insert_segment.end_point;
        auto A_point = root->trapezoid_union_data.trapezoid.left_upper;
        auto B_point = root->trapezoid_union_data.trapezoid.right_upper;
        auto C_point = root->trapezoid_union_data.trapezoid.left_lower;
        auto D_point = root->trapezoid_union_data.trapezoid.right_lower;
        auto E_point = left_point;
        auto F_point = right_point;
        //            A---------------------------------B
        //            |    S                            |
        //            |                                 |                        EF_segment_node
        //    E-------J---------------------------------K--------F                /         \
        //            |                                 |                        /           \
        //            |    T                            |                 S_trapezoid         T_trapezoid
        //            C---------------------------------D
        auto J_point = Segment<Point_2>::get_segment_point_on_x(E_point, F_point, C_point.x);
        auto K_point = Segment<Point_2>::get_segment_point_on_x(E_point, F_point, D_point.x);
        auto S_trapezoid = init_four_points(A_point, B_point, J_point, K_point);
        auto T_trapezoid = init_four_points(J_point, K_point, C_point, D_point);
        auto EF_segment_node = init_segment_node(E_point, F_point);
        trapezoid_graph_Node::replace_sub_tree_left(EF_segment_node, S_trapezoid);
        trapezoid_graph_Node::replace_sub_tree_right(EF_segment_node, T_trapezoid);

        return EF_segment_node;
    }

    static trapezoid_ptr init_root(AABB<Point_2> bounding_box) {
        auto result_ptr = new trapezoid_graph_Node;
        result_ptr->trapezoid_union_data.trapezoid.left_upper = {
            bounding_box.min_point.x, bounding_box.max_point.y
        };
        result_ptr->trapezoid_union_data.trapezoid.right_upper = {
            bounding_box.max_point.x, bounding_box.max_point.y
        };
        result_ptr->trapezoid_union_data.trapezoid.left_lower = {
            bounding_box.min_point.x, bounding_box.min_point.y
        };
        result_ptr->trapezoid_union_data.trapezoid.right_lower = {
            bounding_box.max_point.x, bounding_box.min_point.y
        };
        result_ptr->trapezoid_type = graph_enum::leaf_node;
        return result_ptr;
    }

    static trapezoid_ptr init_four_points(Point_2 A, Point_2 B, Point_2 C, Point_2 D) {
        auto result_ptr = new trapezoid_graph_Node;
        result_ptr->trapezoid_union_data.trapezoid.left_upper = {A};
        result_ptr->trapezoid_union_data.trapezoid.right_upper = {B};
        result_ptr->trapezoid_union_data.trapezoid.left_lower = {C};
        result_ptr->trapezoid_union_data.trapezoid.right_lower = {D};
        result_ptr->trapezoid_type = graph_enum::leaf_node;
        return result_ptr;
    }

    static trapezoid_ptr init_points_node(Point_2 A) {
        auto result_ptr = new trapezoid_graph_Node;
        result_ptr->trapezoid_union_data.segment_point = A;
        result_ptr->trapezoid_type = graph_enum::point_node;
        return result_ptr;
    }

    static trapezoid_ptr init_segment_node(Point_2 A, Point_2 B) {
        auto result_ptr = new trapezoid_graph_Node<T>;
        result_ptr->trapezoid_union_data.segment.start_point = A;
        result_ptr->trapezoid_union_data.segment.end_point = B;
        result_ptr->trapezoid_type = graph_enum::segment_node;
        return result_ptr;
    }
};

#endif //TRAPEZOID_H
