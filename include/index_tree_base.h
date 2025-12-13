//
// Created by 潘鑫 on 2025/11/16.
//

#ifndef INDEX_BINARY_TREE_NODE_H
#define INDEX_BINARY_TREE_NODE_H
#include <vector>

#include "tree_node.h"
#include "tree_function.h"
#include "tree_node_index.h"
#include "index_binary_tree_node.h"


template<typename T, typename index_node>
class index_tree_base {
public:
    using node = index_node;
    std::vector<v_index> roots;
    std::vector<node> details;
    v_index delete_v_index;

    node *get_node_from_v_index(v_index in) {
        if (in == get_nil_index()) {
            return nullptr;
        }
        return &details.at(in);
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

    std::vector<T> *translate_data(std::vector<v_index> src_s) {
        auto result = new std::vector<T>();
        for (auto src: src_s) {
            result->push_back(get_node_ptr(src)->data);
        }
        return result;
    }

    v_index tree_find_value(T data) {
        return BIN_tree::tree_find_value(get_root_index(), data,
                                         get_nil_index(),
                                         std::bind(&index_tree_base::get_node_ptr, this, std::placeholders::_1));
    }

    std::vector<v_index> *inorder_tree_walk_with_stack(std::vector<v_index> *result) {
        return tree_walk_with_stack(get_root_index(), result,
                                    get_nil_index(),
                                    std::bind(&index_tree_base::get_node_ptr, this,
                                              std::placeholders::_1), tree_walk_type::inorder_type);
    }


    std::vector<v_index> *preorder_tree_walk_index() {
        auto result = new std::vector<v_index>();
        auto last_result = tree_walk_with_stack(get_root_index(),
                                                result,
                                                get_nil_index(),
                                                std::bind(&index_tree_base::get_node_ptr, this,
                                                          std::placeholders::_1),
                                                tree_walk_type::preorder_type);
        return last_result;
    }


    node *get_node_ptr(v_index need) {
        if (need == get_nil_index()) {
            return nullptr;
        }
        return &details.at(need);
    }

    node *get_root_node() {
        return &details.at(get_root_index());
    }

    v_index get_nil_index() {
        return {0};
    }

    // 这里的本质是一个链表，虽然已经使用过了，但是并不删除，只是标记并没有被使用，新插入时占据原本的位置
    bool update_new_delete_node(v_index delete_node_index) {
        get_node_ptr(delete_node_index)->left = delete_v_index;
        delete_v_index = delete_node_index;
        return true;
    }

    // 这里的本质是一个链表，虽然已经使用过了，但是并不删除，只是标记并没有被使用，新插入时占据原本的位置
    v_index get_one_delete_node() {
        auto new_node_v_index = delete_v_index;
        delete_v_index = get_node_ptr(new_node_v_index)->left;
        return new_node_v_index;
    }

    v_index get_new_node_index(T input_data) {
        if (roots.empty() == true && details.empty() == true) {
            init_root(input_data);
            delete_v_index = get_nil_index();
        }

        v_index new_node_v_index;
        if (delete_v_index == get_nil_index()) {
            node new_node{input_data, get_nil_index()};
            new_node_v_index = details.size();
            details.push_back(new_node);
        } else {
            new_node_v_index = get_one_delete_node();
            index_node::init_index_node(get_node_ptr(new_node_v_index),
                                        input_data,
                                        get_nil_index());
        }
        return new_node_v_index;
    }

    T *get_data(v_index index) {
        auto temp = get_node_from_v_index(index);
        if (temp != nullptr)
            return &temp->data;
        return nullptr;
    }

    v_index predecessor(v_index index) {
        return BIN_tree::tree_predecessor(index,
                                          get_nil_index(),
                                          std::bind(&index_tree_base::get_node_ptr, this,
                                                    std::placeholders::_1));
    }

    const node *predecessor_node(v_index index) {
        return get_node_from_v_index(predecessor(index));
    }

    const node *successor_node(v_index index) {
        return get_node_from_v_index(successor(index));
    }

    const T *predecessor_data(v_index index) {
        return get_data(predecessor(index));
    }

    const T *successor_data(v_index index) {
        return get_data(successor(index));
    }


    v_index successor(v_index index) {
        return BIN_tree::tree_successor(index,
                                        get_nil_index(),
                                        std::bind(&index_tree_base::get_node_ptr, this,
                                                  std::placeholders::_1));
    }

    v_index tree_find_index(T input_data) {
        return tree_find_value(input_data);
    }

    node *tree_find_node(T input_data) {
        return get_node_from_v_index(tree_find_value(input_data));
    }

    T *tree_find_data(T input_data) {
        return get_data(tree_find_value(input_data));
    }

    v_index minimum(v_index index) {
        return BIN_tree::tree_minimum(index,
                                      get_nil_index(),
                                      std::bind(&index_tree_base::get_node_ptr, this,
                                                std::placeholders::_1));
    }

    const node *minimum_node(v_index index) {
        return get_node_from_v_index(minimum_node(index));
    }

    const T *minimum_data(v_index index) {
        return get_data(minimum(index));
    }

    const T *tree_minimum_data() {
        return get_data(minimum(get_root_index()));
    }

    v_index tree_minimum_index() {
        return minimum(get_root_index());
    }

    const node *tree_minimum_data_node() {
        return get_node_from_v_index(get_root_index());
    }
};


// #undef v_index

#endif //INDEX_BINARY_TREE_NODE_H
