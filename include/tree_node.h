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

    T *tree_successor(T *tree_node) {
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

    T *tree_minimum(T *tree_node) {
        T *return_node = nullptr;
        while (tree_node != nullptr) {
            return_node = tree_node;
            tree_node = tree_node->left;
        }
        return return_node;
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
            temp_node_right->parent = node->parent;
            if (node->parent == nullptr) {
            } else if (node == node->parent->left) {
                node->parent->left = temp_node_right;
            } else if (node == node->parent->right) {
                node->parent->right = temp_node_right;
            }
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
            temp_node_left->parent = node->parent;
            if (node->parent == nullptr) {
            } else if (node == node->parent->left) {
                node->parent->left = temp_node_left;
            } else if (node == node->parent->right) {
                node->parent->right = temp_node_left;
            }
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
