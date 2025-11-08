//
// Created by 潘鑫 on 2025/9/24.
//

#ifndef TREE_NODE_H
#define TREE_NODE_H
#include <iostream>

template<class T>
class Tree_Node {
public:
    T *parent;
    T *left;
    T *right;

    static T *tree_successor(T *tree_node) {
        if (tree_node->right != nullptr) {
            return tree_minimum(tree_node->right);
        }
        T *result_node = tree_node->parent;
        while (result_node != nullptr && result_node->right == tree_node) {
            tree_node = result_node;
            result_node = result_node->parent;
        }
        return result_node;
    }

    T *tree_predecessor(T *tree_node) {
        if (tree_node->left != nullptr) {
            return tree_maximum(tree_node->left);
        }
        T *result_node = tree_node->parent;
        while (result_node != nullptr && result_node->left == tree_node) {
            tree_node = result_node;
            result_node = result_node->parent;
        }
        return result_node;
    }

    static T *tree_minimum(T *tree_node) {
        T *return_node = nullptr;
        while (tree_node != nullptr) {
            return_node = tree_node;
            tree_node = tree_node->left;
        }
        return return_node;
    }

    static T *replace_sub_tree(T *dst_sub_tree_position, T *src_sub_tree) {
        src_sub_tree->parent = dst_sub_tree_position->parent;
        if (src_sub_tree->parent == nullptr) {
        } else if (dst_sub_tree_position == src_sub_tree->parent->left) {
            src_sub_tree->parent->left = src_sub_tree;
        } else if (dst_sub_tree_position == src_sub_tree->parent->right) {
            src_sub_tree->parent->right = src_sub_tree;
        }
        return src_sub_tree;
    }

    static T *replace_sub_tree_left(T *sub_tree, T *new_left_sub_tree) {
        sub_tree->left = new_left_sub_tree;
        if (new_left_sub_tree != nullptr)
            new_left_sub_tree->parent = sub_tree;
        return sub_tree;
    }

    static T *replace_sub_tree_right(T *sub_tree, T *new_right_sub_tree) {
        sub_tree->right = new_right_sub_tree;
        if (new_right_sub_tree != nullptr)
            new_right_sub_tree->parent = sub_tree;
        return sub_tree;
    }

    static T *clean_sub_tree_father(T *need_clean_sub_tree) {
        if (need_clean_sub_tree->parent == nullptr) {
        } else if (need_clean_sub_tree == need_clean_sub_tree->parent->left) {
            need_clean_sub_tree->parent->left = nullptr;
        } else if (need_clean_sub_tree == need_clean_sub_tree->parent->right) {
            need_clean_sub_tree->parent->right = nullptr;
        }
        need_clean_sub_tree->parent = nullptr;
        return need_clean_sub_tree;
    }

    bool left_rotate(T *node) {
        if (node != nullptr && node->right == nullptr) {
            return false;
        } else {
            auto temp_node_right = node->right;
            node->right = node->right->left;
            if (node->right != nullptr) {
                node->right->parent = node; // 还需要判空
            }
            replace_sub_tree(node, temp_node_right);
            temp_node_right->parent = node->parent;
            temp_node_right->left = node;
            node->parent = temp_node_right;
        }
    }

    static bool right_rotate(T *node) {
        if (node != nullptr && node->left == nullptr) {
            return false;
        } else {
            auto temp_node_left = node->left;
            node->left = temp_node_left->right;
            if (node->left != nullptr) {
                node->left->parent = node;
            }
            replace_sub_tree(node, temp_node_left);
            temp_node_left->right = node;
            node->parent = temp_node_left;
        }
    }

    T *tree_maximum(T *tree_node) {
        T *return_node = nullptr;
        while (tree_node != nullptr) {
            return_node = tree_node;
            tree_node = tree_node->right;
        }
        return return_node;
    }

    T *find_root(T *node) {
        if (node == nullptr) {
            return nullptr;
        } else {
            while (node->parent != nullptr) {
                return find_root(node->parent);
            }
            return node;
        }
    }


    /**
     * 这个方法可以从扩展类中移动到基类当中
     * @param root
     * @return
     */
    T *find_miximum_leaf(T *root) {
        if (root == nullptr) {
            return nullptr;
        } else {
            while (root->right != nullptr || root->left != nullptr) {
                if (root->right != nullptr) {
                    return find_miximum_leaf(root->right);
                } else {
                    return find_miximum_leaf(root->left);
                }
            }
            return root;
        }
    }
};

#endif //TREE_NODE_H
