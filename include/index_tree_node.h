//
// Created by 潘鑫 on 2025/11/18.
//

#ifndef INDEX_TREE_NODE_H
#define INDEX_TREE_NODE_H

#include "tree_node_index.h"

template<class T>
class index_Tree_Node {
    using ptr = T *;

public:
    v_index left;
    v_index right;
    v_index parent;

public:
    T data;

    index_Tree_Node(T _data, v_index _nil_v_index) {
        data = _data;
        left = _nil_v_index;
        right = _nil_v_index;
        parent = _nil_v_index;
    }
};
#endif //INDEX_TREE_NODE_H
