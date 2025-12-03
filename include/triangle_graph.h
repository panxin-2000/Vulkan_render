//
// Created by 潘鑫 on 2025/12/3.
//

#ifndef HELLO_MAC_TRIANGLE_GRAPH_H
#define HELLO_MAC_TRIANGLE_GRAPH_H
#include "half_edge.h"
#include "vector_signed_area.h"

template<typename T>
class Triangle_node {
    Triangle<T> triangle;
    int face_index = 0;
    Triangle_node *inner_triangle[3];

public:
    Triangle_node(T a_1, T b_1, T c_1, int face_index) : triangle(a_1, b_1, c_1), face_index(face_index) {
    }

    void add_triangle_node(Triangle_node *node) {
        for (int i = 0; i < 3; ++i) {
            if (node->inner_triangle[i] != nullptr) {
                node->inner_triangle[i] = node;
                face_index = -1;
                break;
            }
        }
        assert(false && "inner_triangle[3] has full");
    }
};

template<typename T>
class Triangle_node_tree {
    Triangle_node<T> *root_triangle;
    // 这里可以肯定的是root是不会变的。

    // 翻转边的时候需要做什么呢？
    // 找到两个原本的三角形，
    //     怎么找到原本的两个三角形呢？
    //     能拿到原本的两个三角形的 face_index，能拿到中点
    //     在未 flip edge 之前是能拿到两个三角形的指针的。flip edge之后其实也是能够拿到的
    //     face_index 在 flip edge 前后也是没有改变的，flip edge之后 new_三角形的中点改变了
    //     有办法能算出原本的，但是之前能拿到更好，具体写的时候看看吧。
    // 在它们内部各添加两个三角形
    // face_index是会被清零的或者说置为-1

    Triangle_node<T> *find_triangle_node(T point) {
    }

    void add_split_point() {
        // 找到重心，
        // 从根结点开始遍历找到三角形
        // 将三角形进行分裂
    }
};


#endif //HELLO_MAC_TRIANGLE_GRAPH_H
