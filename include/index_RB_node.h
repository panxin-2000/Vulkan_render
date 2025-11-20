//
// Created by 潘鑫 on 2025/11/21.
//

#ifndef INDEX_RB_NODE_H
#define INDEX_RB_NODE_H
#include "index_binary_tree_node.h"


template<typename T, typename index_node>
class index_RB_tree : public index_binary_Tree_Node<T, index_node> {
    using node = index_node;

public:
    v_index tree_insert_value(v_index root, T data) {
        this->add_new_node(data);
        return root;
    }

    v_index add_new_node(T input_data) {
        if (index_binary_Tree::roots.empty() == true && index_binary_Tree::details.empty() == true) {
            index_binary_Tree::init_root(input_data);
        }
        node new_node{input_data, index_binary_Tree::get_nil_index()};

        v_index new_node_v_index;
        new_node_v_index.number = index_binary_Tree::details.size();
        index_binary_Tree::details.push_back(new_node);

        auto temp = BIN_tree::add_new_node(index_binary_Tree::get_root_index(),
                                           new_node_v_index,
                                           index_binary_Tree::get_nil_index(),
                                           std::bind(&index_binary_Tree::get_node_ptr, this,
                                                     std::placeholders::_1));
        index_binary_Tree::roots.push_back(temp);
        return new_node_v_index;
    }


    v_index delete_node(v_index delete_node) {
        return BIN_tree::delete_node_from_binary_search_tree(index_binary_Tree::get_root_index(), delete_node,
                                                             index_binary_Tree::get_nil_index(),
                                                             std::bind(&index_binary_Tree::get_node_ptr, this,
                                                                       std::placeholders::_1));
    }
};

#endif //INDEX_RB_NODE_H
