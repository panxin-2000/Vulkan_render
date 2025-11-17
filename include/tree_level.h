//
// Created by 潘鑫 on 2025/11/17.
//

#ifndef TREE_LEVEL_H
#define TREE_LEVEL_H


template<typename T>
std::vector<T> level_tree_walk(T node, const std::vector<T> *result) {
    return level_tree_walk(node, result, static_cast<T>(nullptr), [](T insert_node) { return insert_node; });
}

template<typename T, typename function>
std::vector<T> level_tree_walk(T node, const std::vector<T> *result, T nil_ptr_or_index, function get_node) {
    std::queue<T> tem;
    if (node != nil_ptr_or_index) {
        tem.push(node);
    }
    while (!tem.empty()) {
        T *node_tem = tem.front();
        if (get_node(node_tem).left != nil_ptr_or_index) {
            tem.push(get_node(node_tem).left);
        }
        if (get_node(node_tem).right != nil_ptr_or_index) {
            tem.push(get_node(node_tem).right);
        }
        result->push_back(node_tem);
        tem.pop();
    }
}

#endif //TREE_LEVEL_H
