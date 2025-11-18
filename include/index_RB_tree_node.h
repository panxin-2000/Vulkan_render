//
// Created by 潘鑫 on 2025/11/18.
//

#ifndef INDEX_RB_TREE_NODE_H
#define INDEX_RB_TREE_NODE_H


#include "tree_node_index.h"

template<class T>
class index_Tree_Node {
    using ptr = T *;

public:
    v_index left;
    v_index right;
    v_index parent;

    enum RB_Tree_Node_color {
        RB_Tree_BLACK = 0,
        RB_Tree_RED = 1,
    };

    RB_Tree_Node_color color;

public:
    T data;


    index_Tree_Node(T _data, v_index _nil_v_index) {
        data = _data;
        left = _nil_v_index;
        right = _nil_v_index;
        parent = _nil_v_index;
        color = RB_Tree_RED;// 默认是红色好一点，插入的时候比较方便发现问题
    }
};


#endif //INDEX_RB_TREE_NODE_H
