//
// Created by 潘鑫 on 2025/2/17.
//

#ifndef HELLO_MAC_TREE_H
#define HELLO_MAC_TREE_H


struct Tree_Node {
    struct Tree_Node *parent;
    struct Tree_Node *left;
    struct Tree_Node *right;
    int value;
};

struct Tree_Node *tree_search(struct Tree_Node *tree_node, int value);
struct Tree_Node *iterative_tree_search(struct Tree_Node *tree_node, int value);
struct Tree_Node *tree_maximum(struct Tree_Node *tree_node);
struct Tree_Node *tree_minimum(struct Tree_Node *tree_node);
struct Tree_Node *tree_successor(struct Tree_Node *tree_node);
struct Tree_Node *tree_predecessor(struct Tree_Node *tree_node);
struct Tree_Node *tree_insert(struct Tree_Node *root_node, struct Tree_Node &insert_tree_node);
struct Tree_Node *tree_insert_value(struct Tree_Node *root_node, int value);

void inorder_tree_walk(struct Tree_Node *node);
void preorder_tree_walk(struct Tree_Node *node);
void postorder_tree_walk(struct Tree_Node *node);
void level_tree_walk(struct Tree_Node *node);

#endif //HELLO_MAC_TREE_H
