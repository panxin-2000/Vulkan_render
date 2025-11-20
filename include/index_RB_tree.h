//
// Created by 潘鑫 on 2025/11/21.
//

#ifndef INDEX_RB_NODE_H
#define INDEX_RB_NODE_H
#include "index_tree_base.h"


template<typename T, typename index_node>
class index_RB_tree : public index_tree_base<T, index_node> {
    using node = index_node;

public:
    v_index tree_insert_value(v_index root, T data) {
        this->add_new_node(data);
        return root;
    }


    v_index add_new_node(T input_data) {
        auto new_node_v_index = index_RB_tree::get_new_node_index(input_data);
        auto temp = BIN_tree::add_new_node(index_RB_tree::get_root_index(),
                                           new_node_v_index,
                                           index_RB_tree::get_nil_index(),
                                           std::bind(&index_RB_tree::get_node_ptr, this,
                                                     std::placeholders::_1));
        index_RB_tree::roots.push_back(temp);
        return new_node_v_index;
    }


    v_index delete_node(v_index delete_node) {
        auto temp_index = BIN_tree::delete_node_from_binary_search_tree(index_RB_tree::get_root_index(),
                                                                        delete_node,
                                                                        index_RB_tree::get_nil_index(),
                                                                        std::bind(&index_RB_tree::get_node_ptr,
                                                                            this,
                                                                            std::placeholders::_1));
        index_RB_tree::update_new_delete_node(temp_index);
        return temp_index;
    }
};

#endif //INDEX_RB_NODE_H
