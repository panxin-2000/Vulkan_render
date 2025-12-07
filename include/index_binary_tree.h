//
// Created by 潘鑫 on 2025/11/21.
//

#ifndef INDEX_BINARY_TREE_H
#define INDEX_BINARY_TREE_H
#include "index_tree_base.h"

template<typename T, typename index_node>
class index_binary_Tree : public index_tree_base<T, index_node> {
    using node = index_node;

public:
    v_index tree_insert_value(v_index root, T data) {
        this->add_new_node(data);
        return root;
    }

    T *get_data(v_index index) {
        auto temp = index_binary_Tree::get_node_from_v_index(index);
        if (temp != nullptr)
            return &temp->data;
        return nullptr;
    }

    v_index predecessor(v_index index) {
        return BIN_tree::tree_predecessor(index,
                                          index_binary_Tree::get_nil_index(),
                                          std::bind(&index_binary_Tree::get_node_ptr, this,
                                                    std::placeholders::_1));
    }

    const node *predecessor_node(v_index index) {
        return index_binary_Tree::get_node_from_v_index(predecessor(index));
    }

    const node *successor_node(v_index index) {
        return index_binary_Tree::get_node_from_v_index(successor(index));
    }

    const T *predecessor_data(v_index index) {
        return get_data(predecessor(index));
    }

    const T *successor_data(v_index index) {
        return get_data(successor(index));
    }


    v_index successor(v_index index) {
        return BIN_tree::tree_successor(index,
                                        index_binary_Tree::get_nil_index(),
                                        std::bind(&index_binary_Tree::get_node_ptr, this,
                                                  std::placeholders::_1));
    }


    node *tree_find_node(T input_data) {
        return index_binary_Tree::get_node_from_v_index(index_binary_Tree::tree_find_value(input_data));
    }

    T *tree_find_data(T input_data) {
        return get_data(index_binary_Tree::tree_find_value(input_data));
    }

    v_index minimum(v_index index) {
        return BIN_tree::tree_minimum(index,
                                      index_binary_Tree::get_nil_index(),
                                      std::bind(&index_binary_Tree::get_node_ptr, this,
                                                std::placeholders::_1));
    }

    const node *minimum_node(v_index index) {
        return index_binary_Tree::get_node_from_v_index(minimum_node(index));
    }

    const T *minimum_data(v_index index) {
        return get_data(minimum(index));
    }

    const T *tree_minimum_data() {
        return get_data(minimum(index_binary_Tree::get_root_index()));
    }

    const node *tree_minimum_data_node() {
        return index_binary_Tree::get_node_from_v_index(index_binary_Tree::get_root_index());
    }

    v_index add_new_node(T input_data) {
        auto new_node_v_index = index_binary_Tree::get_new_node_index(input_data);
        auto temp = BIN_tree::add_new_node(index_binary_Tree::get_root_index(),
                                           new_node_v_index,
                                           index_binary_Tree::get_nil_index(),
                                           std::bind(&index_binary_Tree::get_node_ptr, this,
                                                     std::placeholders::_1));
        index_binary_Tree::roots.push_back(temp);
        return new_node_v_index;
    }

    void pop_minimum() {
        delete_node(minimum(index_binary_Tree::get_root_index()));
    }


    v_index delete_node(v_index delete_node) {
        if (delete_node == index_binary_Tree::get_nil_index())
            return delete_node;
        auto root_index = BIN_tree::delete_node_from_binary_search_tree(index_binary_Tree::get_root_index(),
                                                                        delete_node,
                                                                        index_binary_Tree::get_nil_index(),
                                                                        std::bind(&index_binary_Tree::get_node_ptr,
                                                                            this,
                                                                            std::placeholders::_1));
        index_binary_Tree::update_new_delete_node(delete_node);
        index_binary_Tree::roots.push_back(root_index);
        return delete_node;
    }
};
#endif //INDEX_BINARY_TREE_H
