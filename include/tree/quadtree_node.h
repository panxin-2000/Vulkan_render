//
// Created by 潘鑫 on 2025/12/19.
//

#ifndef HELLO_MAC_QUADTREE_NODE_H
#define HELLO_MAC_QUADTREE_NODE_H

#include "iostream"

/**
 *
 * @tparam T Point_2 或者 point_3 ,表示二维或者三维
 * @tparam number
 */
template<class T, size_t number = 4 + 1>
class Quad_Tree_Node {
    using ptr = T *;

public:
    u_int16_t node[number];
    T centroid_point; // 重心
    T direction_interval; // 方向间隔
    std::vector<u_int16_t> AABB_index;

public:
    Quad_Tree_Node() = default;

    Quad_Tree_Node(T centroid_point, T direction_interval) : centroid_point(centroid_point),
                                                             direction_interval(direction_interval) {
        for (int i = 0; i < number; i++) {
            node[0] = 0;
        }
    };



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
