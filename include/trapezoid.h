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
    TRAPEZOID trapezoid;
    // 这里有一个使用模版不断套娃的小技巧
public:
    static std::unique_ptr<trapezoid_graph_Node<T> > init_root(AABB<point_2> bounding_box) {
        std::unique_ptr<trapezoid_graph_Node<T> > result_ptr = std::make_unique<trapezoid_graph_Node<T> >();
        result_ptr->trapezoid.left_upper = {bounding_box.min_point.x, bounding_box.max_point.y};
        result_ptr->trapezoid.right_upper = {bounding_box.max_point.x, bounding_box.max_point.y};
        result_ptr->trapezoid.left_lower = {bounding_box.min_point.x, bounding_box.min_point.y};
        result_ptr->trapezoid.right_lower = {bounding_box.max_point.x, bounding_box.min_point.y};
        return result_ptr;
    }
};

#endif //TRAPEZOID_H
