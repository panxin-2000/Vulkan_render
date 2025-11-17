//
// Created by 潘鑫 on 2025/11/17.
//

#ifndef TREE_POSTORDER_H
#define TREE_POSTORDER_H
#include "tree_order.h"

template<typename T>
void postorder_tree_walk(T node, std::vector<T> *result) {
    postorder_tree_walk(node, result,
                        static_cast<T>(nullptr),
                        [](T insert_node) { return *insert_node; });
}

template<typename T, typename function>
void postorder_tree_walk(T node, std::vector<T> *result, T nil_ptr_or_index, function get_node) {
    if (node != nil_ptr_or_index) {
        postorder_tree_walk(get_node(node).left, result);
        postorder_tree_walk(get_node(node).right, result);
        result->push_back(node);
    }
}


template<typename T>
std::vector<T> *postorder_tree_walk_with_stack(T root, std::vector<T> *result) {
    return tree_walk_with_stack(root, result,
                                static_cast<T>(nullptr),
                                [](T insert_node) { return *insert_node; },
                                tree_walk_type::postorder_type);
}


#endif //TREE_POSTORDER_H
