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
        T *y = tree_node->parent;
        while (y != nullptr && y->right == tree_node) {
            tree_node = y;
            y = y->parent;
        }
        return tree_node;
    }

    T *tree_predecessor(T *tree_node) {
        if (tree_node->left != nullptr) {
            return tree_minimum(tree_node->left);
        }
        T *y = tree_node->parent;
        while (y != nullptr && y->left == tree_node) {
            tree_node = y;
            y = y->parent;
        }
        return tree_node;
    }

    T *tree_minimum(T *tree_node) {
        T *return_node = nullptr;
        while (tree_node != nullptr) {
            return_node = tree_node;
            tree_node = tree_node->left;
        }
        return return_node;
    }

    T *tree_maximum(T *tree_node) {
        while (tree_node != nullptr) {
            tree_node = tree_node->right;
        }
        return tree_node;
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
