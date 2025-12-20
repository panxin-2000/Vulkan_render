//
// Created by 潘鑫 on 2025/12/19.
//

#ifndef HELLO_MAC_AABB_TREE_H
#define HELLO_MAC_AABB_TREE_H


#include "index_tree_base.h"
#include "quadtree_node.h"

// 还好，只是要做AABB的二叉树，第一个问题事，如果AABB树相交应该怎么办


template<typename T, typename index_node>
class aabb_tree : public index_tree_base<T, index_node> {
    using node = index_node;

public:
    v_index tree_insert_value(v_index root, T data) {
        this->add_new_node(data);
        return root;
    }

    v_index add_new_node(T input_data) {
        auto new_node_v_index = aabb_tree::get_new_node_index(input_data);
        // 查看根结点的内容，位置和大小？
        // 然后知道当前结点的位置和大小
        // 大小用于判断当前应该在哪一个层
        // 位置用于判断在四叉树的那个格子中
        // auto temp = BIN_tree::add_new_node(aabb_tree::get_root_index(),
        //                                    new_node_v_index,
        //                                    aabb_tree::get_nil_index(),
        //                                    std::bind(&aabb_tree::get_node_ptr, this,
        //                                              std::placeholders::_1));
        aabb_tree::roots.push_back(temp);
        return new_node_v_index;
    }

    v_index delete_node(v_index delete_node) {
        if (delete_node == aabb_tree::get_nil_index())
            return delete_node;
        // auto root_index = BIN_tree::delete_node_from_binary_search_tree(aabb_tree::get_root_index(),
        //                                                                 delete_node,
        //                                                                 aabb_tree::get_nil_index(),
        //                                                                 std::bind(&aabb_tree::get_node_ptr,
        //                                                                     this,
        //                                                                     std::placeholders::_1));
        aabb_tree::update_new_delete_node(delete_node);
        aabb_tree::roots.push_back(root_index);
        return delete_node;
    }
};

template<typename FT>
using AABB_Tree = aabb_tree<FT, Quad_Tree_Node<FT> >;


#endif //HELLO_MAC_AABB_TREE_H
