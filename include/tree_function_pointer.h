//
// Created by 潘鑫 on 2025/11/21.
//

#ifndef TREE_FUNCTION_POINTER_H
#define TREE_FUNCTION_POINTER_H
#include <vector>

namespace BIN_tree {
    template<typename ptr>
    ptr replace_sub_tree_right(ptr sub_tree, ptr new_right_sub_tree) {
        replace_sub_tree_right(sub_tree, new_right_sub_tree,
                               static_cast<ptr>(nullptr),
                               [](ptr insert_node) { return insert_node; });
    }

    template<typename ptr>
    static bool left_rotate(ptr node) {
        left_rotate(node, static_cast<ptr>(nullptr),
                    [](ptr insert_node) { return insert_node; });
    }

    template<typename ptr>
    ptr find_root(ptr node) {
        return find_root(node,
                         static_cast<ptr>(nullptr),
                         [](ptr insert_node) { return insert_node; });
    }

    template<typename RB_Tree_Node>
    RB_Tree_Node left_rotate_with_color(RB_Tree_Node node) {
        return left_rotate_with_color(node, static_cast<RB_Tree_Node>(nullptr),
                                      [](RB_Tree_Node insert_node) { return insert_node; });
    }

    template<typename RB_Tree_Node>
    RB_Tree_Node right_rotate_with_color(RB_Tree_Node node) {
        return right_rotate_with_color(node,
                                       static_cast<RB_Tree_Node>(nullptr),
                                       [](RB_Tree_Node insert_node) { return insert_node; });
    }






    template<typename T>
    T preorder_tree_walk_with_stack_find_father(T root, T node) {
        preorder_tree_walk_with_stack_find_father(root, node, static_cast<T>(nullptr),
                                                  [](T insert_node) { return insert_node; });
    }

    template<typename T>
    T preorder_tree_walk_find_father(T root, T node) {
        return preorder_tree_walk_find_father(root, node,
                                              static_cast<T>(nullptr),
                                              [](T insert_node) { return insert_node; });
    }



    // T2 RB_Tree_Node<segment_vector>   T  segment_vector
    // 这个函数中找到的是值，时间上如果能够返回



    template<typename ptr>
    static ptr replace_sub_tree(ptr dst_sub_tree_position, ptr src_sub_tree) {
        return replace_sub_tree(dst_sub_tree_position, src_sub_tree,
                                static_cast<ptr>(nullptr),
                                [](ptr insert_node) { return insert_node; });
    }

    template<typename ptr>
    ptr replace_sub_tree_left(ptr sub_tree, ptr new_left_sub_tree) {
        replace_sub_tree_left(sub_tree, new_left_sub_tree,
                              static_cast<ptr>(nullptr),
                              [](ptr insert_node) { return insert_node; });
    }
}
#endif //TREE_FUNCTION_POINTER_H
