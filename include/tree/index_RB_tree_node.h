//
// Created by 潘鑫 on 2025/11/18.
//

#ifndef INDEX_RB_TREE_NODE_H
#define INDEX_RB_TREE_NODE_H


#include "tree_node_index.h"

template<class T>
class index_RB_Tree_Node {
    using ptr = T *;

public:
    v_index left: 21;
    v_index right: 21;
    v_index parent: 21;


    v_index color: 1;

public:
    T data;


    index_RB_Tree_Node(T _data, v_index _nil_v_index) {
        data = _data;
        left = _nil_v_index;
        right = _nil_v_index;
        parent = _nil_v_index;
        color = RB_Tree_RED; // 默认是红色好一点，插入的时候比较方便发现问题
    }

    static bool init_index_node(index_RB_Tree_Node *address, T _data, v_index _nil_v_index) {
        address->data = _data;
        address->left = _nil_v_index;
        address->right = _nil_v_index;
        address->parent = _nil_v_index;
        address->color = RB_Tree_RED;
        return true;
    }
};


#endif //INDEX_RB_TREE_NODE_H
