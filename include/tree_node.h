//
// Created by 潘鑫 on 2025/9/24.
//

#ifndef TREE_NODE_H
#define TREE_NODE_H
#include "add_char_calculata.h"
#include "add_char_calculata.h"

template<class T>
struct RB_Tree_Node {
public:
    RB_Tree_Node() {
    }

    ~RB_Tree_Node(void) {
    }

    RB_Tree_Node *left; // 因为是64位的，所以指针占据了8个字节
    RB_Tree_Node *right;
    RB_Tree_Node *parent;
    T data;
    int color_tag;

public:
    struct RB_Tree_Node<T> * find_miximum_leaf(struct RB_Tree_Node *root) {
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


    struct RB_Tree_Node *find_root(struct RB_Tree_Node *node) {
        if (node == nullptr) {
            return nullptr;
        } else {
            while (node->parent != nullptr) {
                return find_root(node->parent);
            }
            return node;
        }
    }

    int tree_add_after_node(struct RB_Tree_Node *root, struct RB_Tree_Node *new_node) {
        if (root == nullptr) {
            root = new_node; // 这里是添加首个数字的位置
        } else {
            struct RB_Tree_Node *new_root = root;
            struct RB_Tree_Node *miximum_leaf = new_node->find_miximum_leaf(new_root);
            while (miximum_leaf->parent->right != nullptr) {
                miximum_leaf = miximum_leaf->parent;
            }
            add_new_node_to_right(miximum_leaf->parent, new_node);
        }
    }

    int add_node_to_left_child(struct RB_Tree_Node *left) {
        this->left = left;
        left->parent = this;
    }

    int add_node_to_right_child(struct RB_Tree_Node *right) {
        this->right = right;
        right->parent = this;
    }

    int add_new_node_before_parent(struct RB_Tree_Node *node, struct RB_Tree_Node *new_node) {
        new_node->parent = node->parent;
        new_node->left = node;
        node->parent = new_node;
        if (new_node->parent != nullptr && new_node->parent->left == node) {
            new_node->parent->left = new_node;
        } else if (new_node->parent != nullptr && new_node->parent->right == node) {
            new_node->parent->right = new_node;
        }
    }

    int add_new_node_to_right(struct RB_Tree_Node *node, struct RB_Tree_Node *new_node) {
        new_node->right = node->right; // 新节点的右孩子更新为将要原本的右孩子
        node->right = new_node; // 更新node的右孩子
        new_node->parent = node; // 更新新节点的parent
        if (new_node->right != nullptr) {
            new_node->right->parent = new_node; // 如果原本的右孩子不为空，更新原本的父节点
        }
    }

};


#endif //TREE_NODE_H
