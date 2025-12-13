//
// Created by 潘鑫 on 2025/9/24.
//

#ifndef TREE_NODE_H
#define TREE_NODE_H
#include <iostream>

#include "tree_function.h"
#include "tree_function_pointer.h"

template<class T>
class Tree_Node {
    using ptr = T *;

public:
    ptr parent;
    ptr left;
    ptr right;

    static ptr tree_successor(ptr tree_node) {
        return BIN_tree::tree_successor(tree_node,
                                        static_cast<ptr>(nullptr),
                                        [](ptr insert_node) { return insert_node; });
    }


    static ptr tree_predecessor(ptr tree_node) {
        return BIN_tree::tree_predecessor(tree_node,
                                          static_cast<ptr>(nullptr),
                                          [](ptr insert_node) { return insert_node; });
    }


    static ptr replace_sub_tree(ptr dst_sub_tree_position, ptr src_sub_tree) {
        return BIN_tree::replace_sub_tree(dst_sub_tree_position, src_sub_tree);
    }

    static ptr replace_sub_tree_left(ptr sub_tree, ptr new_left_sub_tree) {
        return BIN_tree::replace_sub_tree_left(sub_tree, new_left_sub_tree);
    }

    static ptr replace_sub_tree_right(ptr sub_tree, ptr new_right_sub_tree) {
        return BIN_tree::replace_sub_tree_right(sub_tree, new_right_sub_tree);
    }


    static ptr clean_sub_tree_father(ptr need_clean_sub_tree) {
        return BIN_tree::clean_sub_tree_father(need_clean_sub_tree,
                                               static_cast<ptr>(nullptr),
                                               [](ptr insert_node) { return insert_node; });
    }


    //            A                              B
    //         E     B                       A       D
    //            C    D                   E   C

    // 原本B是A的右子树，现在变成A是B的左子树
    static bool left_rotate(ptr node) {
        return BIN_tree::left_rotate(node,
                                     static_cast<ptr>(nullptr),
                                     [](ptr insert_node) { return insert_node; });
    }


    //            A                 B
    //         B     E           C     A
    //      C    D                   D   E
    //
    // 原本B是A的左子树，现在变成A是B的右子树
    static bool right_rotate(ptr node) {
        return BIN_tree::right_rotate(node,
                                      static_cast<ptr>(nullptr),
                                      [](ptr insert_node) { return insert_node; });
    }


    ptr find_root(ptr node) {
        return BIN_tree::find_root(node);
    }
};

#endif //TREE_NODE_H
