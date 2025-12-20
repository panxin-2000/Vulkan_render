//
// Created by 潘鑫 on 2025/12/19.
//

#ifndef HELLO_MAC_QUADTREE_NODE_H
#define HELLO_MAC_QUADTREE_NODE_H

#include "iostream"


template<class T, size_t number = 4 + 1>
class Quad_Tree_Node {
    using ptr = T *;

public:
    u_int16_t node[number];

public:
    T data;

    index_Tree_Node(T _data, v_index _nil_v_index) {
        data = _data;
        for (int i = 0; i < number; i++) {
            node[0] = _nil_v_index;
        }
    }

    static bool init_index_node(index_Tree_Node *address, T _data, v_index _nil_v_index) {
        address->data = _data;
        for (int i = 0; i < number; i++) {
            address->node[i] = _nil_v_index;
        }
        return true;
    }
};


#endif //HELLO_MAC_QUADTREE_NODE_H
