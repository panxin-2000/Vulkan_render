//
// Created by 潘鑫 on 2025/11/16.
//

#ifndef INDEX_BINARY_TREE_NODE_H
#define INDEX_BINARY_TREE_NODE_H
#include <vector>

#include "tree_node.h"
#include "tree_function.h"
#include "tree_node_index.h"
#include "index_tree_node.h"


template<typename T, typename index_node>
class index_binary_Tree_Node {
    using node = index_node;
    std::vector<node> details;
    std::vector<v_index> roots;

public:
    index_binary_Tree_Node &get_detail_from_v_index(v_index in) {
        return details.at(in.number);
    }

    // 这里的操作是什么意思呢？
    // 只是为了初始化一个哨兵
    // 其他的操作并没有去做
    v_index init_root(T input_data) {
        node new_node{input_data, get_nil_index()};
        v_index result = get_nil_index();
        details.push_back(new_node);
        roots.push_back(result);
        return result;
    }

    v_index get_root_index() {
        if (roots.empty() == false)
            return roots.at(roots.size() - 1);
        else {
            v_index result = get_nil_index();
            return result;
        }
    }

    std::vector<index_node *> *translate(std::vector<v_index> src_s) {
        auto result = new std::vector<index_node *>();
        for (auto src: src_s) {
            result->push_back(get_node_ptr(src));
        }
        return result;
    }


    std::vector<v_index> *preorder_tree_walk_index() {
        auto result = new std::vector<v_index>();
        auto last_result = tree_walk_with_stack(get_root_index(),
                                                result,
                                                get_nil_index(),
                                                std::bind(&index_binary_Tree_Node::get_node_ptr, this,
                                                          std::placeholders::_1),
                                                tree_walk_type::preorder_type);
        return last_result;
    }

    node &get_node(v_index need) {
        return details.at(need.number);
    }

    node *get_node_ptr(v_index need) {
        return &details.at(need.number);
    }

    node &get_root_node() {
        return details.at(get_root_index().number);
    }

    v_index get_nil_index() {
        v_index result;
        result.number = 0;
        return result;
    }

    v_index tree_insert_value(v_index root, T data) {
        this->add_new_node(data);
        return root;
    }


    v_index add_new_node(T input_data) {
        if (roots.empty() == true && details.empty() == true) {
            init_root(input_data);
        }
        node new_node{input_data, get_nil_index()};

        v_index new_node_v_index;
        new_node_v_index.number = details.size();
        details.push_back(new_node);

        auto temp = BIN_tree::add_new_node(get_root_index(), new_node_v_index,
                                           get_nil_index(),
                                           std::bind(&index_binary_Tree_Node::get_node_ptr, this,
                                                     std::placeholders::_1));
        roots.push_back(temp);
        return new_node_v_index;
    }


    v_index delete_node_from_binary_search_tree(v_index delete_node) {
        return BIN_tree::delete_node_from_binary_search_tree(get_root_index(), delete_node,
                                                             get_nil_index(),
                                                             std::bind(&index_binary_Tree_Node::get_node_ptr, this,
                                                                       std::placeholders::_1));
    }
};


// #undef v_index

#endif //INDEX_BINARY_TREE_NODE_H
