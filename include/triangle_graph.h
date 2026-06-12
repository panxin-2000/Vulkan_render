//
// Created by 潘鑫 on 2025/12/3.
//

#ifndef HELLO_MAC_TRIANGLE_GRAPH_H
#define HELLO_MAC_TRIANGLE_GRAPH_H
#include <cassert>
#include "base_geometry/base.h"
#include "base_geometry/intersect_function.h"

template<typename T>
class Triangle_node {
public:
    Triangle<T> triangle;
    int face_index = 0;
    Triangle_node *inner_triangle[3];

    Triangle_node(T a_1, T b_1, T c_1, int face_index) : triangle(a_1, b_1, c_1), face_index(face_index) {
    }

    void add_triangle_node(Triangle_node *node) {
        for (auto &i: inner_triangle) {
            // 没想到数组也是能够进行for range 的
            if (i == nullptr) {
                i = node;
                face_index = -1;
                return;
            }
        }
        assert(false && "inner_triangle[3] has full");
    }
};

template<typename T>
class Triangle_node_tree {
    Triangle_node<T> *root_triangle;
    // 这里可以肯定的是root是不会变的。

public:
    explicit Triangle_node_tree(Triangle_node<T> *root_triangle) : root_triangle(root_triangle) {
    }

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
        auto temp_root_triangle = root_triangle;
        auto result = root_triangle;

        if (temp_root_triangle != nullptr &&
            is_intersect(temp_root_triangle->triangle, point)) {
            result = temp_root_triangle;
        }
        while (temp_root_triangle != nullptr) {
            int i = 0;
            for (auto check_triangle: temp_root_triangle->inner_triangle) {
                if (check_triangle != nullptr &&
                    is_intersect(check_triangle->triangle, point)) {
                    temp_root_triangle = check_triangle;
                    result = check_triangle;
                    break;
                } else {
                    i++;
                }
            }
            if (i == 3) {
                break;
            }
        }
        return result;
    }

    void add_split_triangle(T find_triangle_point, Triangle_node<T> *node) {
        // 找到重心，也就是find_triangle_point
        // 从根结点开始遍历找到三角形
        auto triangle_node = find_triangle_node(find_triangle_point);
        if (triangle_node != nullptr) {
            triangle_node->add_triangle_node(node);
        }
        // 将三角形进行分裂
    }
};


#endif //HELLO_MAC_TRIANGLE_GRAPH_H
