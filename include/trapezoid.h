//
// Created by 潘鑫 on 2025/11/1.
//

#ifndef TRAPEZOID_H
#define TRAPEZOID_H
#include "bounding_box.h"

// 我希望四边形有什么样的结构呢？
// 四边形肯定是需要有四个顶点的
// 以及四条边，只有四条边就够了吗
// 还需要知道这个四边形属于那个face
// 怎么才能知道属于那个face呢？
// 创建与有向无环图的双向连接
// 或者说，这个结构需要添加到树中
//
struct TRAPEZOID {
    point_2 left_upper;
    point_2 right_upper;
    point_2 left_lower;
    point_2 right_lower;
};


template<class T>
class trapezoid_graph_Node : public Tree_Node<trapezoid_graph_Node<T> > {
    union my_union {
        TRAPEZOID trapezoid;
        point_2 segment_point;
        segment_position segment;

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

    graph_enum trapezoid_type;

    // 这里有一个使用模版不断套娃的小技巧
public:
    trapezoid_graph_Node() {
    }

    ~trapezoid_graph_Node() {
    }


    std::unique_ptr<trapezoid_graph_Node<T> > find_point_in_trapezoid_graph(point_2 find_point) {
        // 既然这里已经看到了查找相关的内容
        // 那么其实需要先构造一个具体的结构
        // 测试一下我这里查找的结果是否是我需要的
        // 既然要做这么一个测试，其实还是只能边做，边更新了
    }

    std::unique_ptr<trapezoid_graph_Node<T> > add_a_segment(segment_position insert_segement) {
        auto start_point_trapezoid = find_point_in_trapezoid_graph(insert_segement.start_point);
        auto end_point_trapezoid = find_point_in_trapezoid_graph(insert_segement.end_point);
        if (start_point_trapezoid != nullptr && end_point_trapezoid != nullptr &&
            start_point_trapezoid == end_point_trapezoid) {
            // 判断是否在一个梯形中，之后 end_point_trapezoid 其实就不用再使用了
        } else if (start_point_trapezoid != nullptr && end_point_trapezoid != nullptr) {
        }
    }

    std::unique_ptr<trapezoid_graph_Node<T> > replace_node_in_one_trapezoid(
        std::unique_ptr<trapezoid_graph_Node<T> > root, segment_position insert_segement) {
        // 最后还是需要返回的，因为根结点可能是会被改变的
        auto left_point = insert_segement.start_point;
        auto right_point = insert_segement.end_point;
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
        auto A_point = trapezoid_union_data.trapezoid.left_upper;
        auto B_point = trapezoid_union_data.trapezoid.right_upper;
        auto C_point = trapezoid_union_data.trapezoid.left_lower;
        auto D_point = trapezoid_union_data.trapezoid.right_lower;
        auto E_point = left_point;
        auto F_point = right_point;
        // 这几个点还需要计算,计算完成了，开始想怎么构造第一个结构了
        auto G_point = segment_position::get_intersection_point(A_point, B_point, E_point.x);
        auto H_point = segment_position::get_intersection_point(A_point, B_point, F_point.x);
        auto I_point = segment_position::get_intersection_point(C_point, D_point, E_point.x);
        auto J_point = segment_position::get_intersection_point(C_point, D_point, H_point.x);
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
        E_node->left = R_trapezoid;
        E_node->right = F_node;
        F_node->right = U_trapezoid;
        F_node->left = EF_segment_node;
        EF_segment_node->left = S_trapezoid;
        EF_segment_node->right = T_trapezoid;
        return E_node;
    }

    static std::unique_ptr<trapezoid_graph_Node<T> > init_root(AABB<point_2> bounding_box) {
        auto result_ptr = std::make_unique<trapezoid_graph_Node>();
        result_ptr->trapezoid_union_data.trapezoid.left_upper = {bounding_box.min_point.x, bounding_box.max_point.y};
        result_ptr->trapezoid_union_data.trapezoid.right_upper = {bounding_box.max_point.x, bounding_box.max_point.y};
        result_ptr->trapezoid_union_data.trapezoid.left_lower = {bounding_box.min_point.x, bounding_box.min_point.y};
        result_ptr->trapezoid_union_data.trapezoid.right_lower = {bounding_box.max_point.x, bounding_box.min_point.y};
        result_ptr->trapezoid_type = graph_enum::leaf_node;
        return result_ptr;
    }

    static std::unique_ptr<trapezoid_graph_Node<T> > init_four_points(point_2 A, point_2 B, point_2 C, point_2 D) {
        std::unique_ptr<trapezoid_graph_Node<T> > result_ptr = std::make_unique<trapezoid_graph_Node<T> >();
        result_ptr->trapezoid_union_data.trapezoid.left_upper = {A};
        result_ptr->trapezoid_union_data.trapezoid.right_upper = {B};
        result_ptr->trapezoid_union_data.trapezoid.left_lower = {C};
        result_ptr->trapezoid_union_data.trapezoid.right_lower = {D};
        result_ptr->trapezoid_type = graph_enum::leaf_node;
        return result_ptr;
    }

    static std::unique_ptr<trapezoid_graph_Node<T> > init_points_node(point_2 A) {
        std::unique_ptr<trapezoid_graph_Node<T> > result_ptr = std::make_unique<trapezoid_graph_Node<T> >();
        result_ptr->trapezoid_union_data.segment_point = A;
        result_ptr->trapezoid_type = graph_enum::point_node;
        return result_ptr;
    }

    static std::unique_ptr<trapezoid_graph_Node<T> > init_segment_node(point_2 A, point_2 B) {
        std::unique_ptr<trapezoid_graph_Node<T> > result_ptr = std::make_unique<trapezoid_graph_Node<T> >();
        result_ptr->trapezoid_union_data.segment_point.x = A;
        result_ptr->trapezoid_union_data.segment_point.y = B;

        result_ptr->trapezoid_type = graph_enum::point_node;
        return result_ptr;
    }
};

#endif //TRAPEZOID_H
