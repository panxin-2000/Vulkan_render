//
// Created by 潘鑫 on 2025/9/24.
//
#include "tree_node.h"
#include <gtest/gtest.h>


TEST(test_tree, test_tree_insert) {
    balance_Tree_Node<int> *root = nullptr;
    //最先考虑的应该是插入,但是过了5个小时才开始看到插入
    root = root->tree_insert_value(root, 5);
    root = root->tree_insert_value(root, 3);
    root = root->tree_insert_value(root, 2);
    root = root->tree_insert_value(root, 4);
    root = root->tree_insert_value(root, 7);
    root = root->tree_insert_value(root, 8);
    EXPECT_EQ(root->data, 5);
    EXPECT_EQ(root->data, 5);
    EXPECT_EQ(root->left->data, 3);
    EXPECT_EQ(root->data, 5);
    EXPECT_EQ(root->left->data, 3);
    EXPECT_EQ(root->left->left->data, 2);
    EXPECT_EQ(root->data, 5);
    EXPECT_EQ(root->left->data, 3);
    EXPECT_EQ(root->left->left->data, 2);
    EXPECT_EQ(root->left->right->data, 4);
    EXPECT_EQ(root->right->data, 7);

    EXPECT_EQ(root->right->data, 7);
    EXPECT_EQ(root->right->right->data, 8);
}

//       5
//   3      7
// 2   4      8
TEST(test_tree, test_tree_delete) {
    balance_Tree_Node<int> *root = nullptr;
    //最先考虑的应该是插入,但是过了5个小时才开始看到插入
    root = root->tree_insert_value(root, 5);
    EXPECT_EQ(root->data, 5);
    root = root->tree_insert_value(root, 3);
    EXPECT_EQ(root->data, 5);
    EXPECT_EQ(root->left->data, 3);
    root = root->tree_insert_value(root, 2);
    EXPECT_EQ(root->data, 5);
    EXPECT_EQ(root->left->data, 3);
    EXPECT_EQ(root->left->left->data, 2);
    root = root->tree_insert_value(root, 4);
    EXPECT_EQ(root->data, 5);
    EXPECT_EQ(root->left->data, 3);
    EXPECT_EQ(root->left->left->data, 2);
    EXPECT_EQ(root->left->right->data, 4);
    root = root->tree_insert_value(root, 7);
    EXPECT_EQ(root->right->data, 7);

    root = root->tree_insert_value(root, 8);
    EXPECT_EQ(root->right->data, 7);
    EXPECT_EQ(root->right->right->data, 8);
    // 这里是上面测试插入部分的代码，应该怎么做呢？
    // 有没有办法不复制一遍呢？

    root = root->delete_node_from_binary_search_tree(root, *root->right->right);
    EXPECT_EQ(root->right->right, nullptr);
}

//       5
//   3      7
// 2   4      8

//       7
//   3      8
// 2   4
TEST(test_tree, test_tree_delete_root) {
    balance_Tree_Node<int> *root = nullptr;
    //最先考虑的应该是插入,但是过了5个小时才开始看到插入
    root = root->tree_insert_value(root, 5);
    root = root->tree_insert_value(root, 3);
    root = root->tree_insert_value(root, 2);
    root = root->tree_insert_value(root, 4);
    root = root->tree_insert_value(root, 7);
    root = root->tree_insert_value(root, 8);

    root = root->delete_node_from_binary_search_tree(root, *root);
    EXPECT_EQ(root->data, 7);
    EXPECT_EQ(root->left->data, 3);
    EXPECT_EQ(root->right->data, 8);

}


//       5
//   3       7
// 2   4   6   10
//            9   11

TEST(test_tree, test_tree_delete_root_2) {
    balance_Tree_Node<int> *root = nullptr;
    //最先考虑的应该是插入,但是过了5个小时才开始看到插入
    root = root->tree_insert_value(root, 5);
    root = root->tree_insert_value(root, 3);
    root = root->tree_insert_value(root, 2);
    root = root->tree_insert_value(root, 4);
    root = root->tree_insert_value(root, 7);
    root = root->tree_insert_value(root, 6);
    root = root->tree_insert_value(root, 10);
    root = root->tree_insert_value(root, 11);
    root = root->tree_insert_value(root, 9);

    root = root->delete_node_from_binary_search_tree(root, *root->right);
    EXPECT_EQ(root->data, 5);
    EXPECT_EQ(root->left->data, 3);
    EXPECT_EQ(root->right->data, 9);

}


//       5
//   3       7
// 2   4   6   15
//            9   19
//             12
//              13
TEST(test_tree, test_tree_delete_root_3) {
    balance_Tree_Node<int> *root = nullptr;
    //最先考虑的应该是插入,但是过了5个小时才开始看到插入
    root = root->tree_insert_value(root, 5);
    root = root->tree_insert_value(root, 3);
    root = root->tree_insert_value(root, 2);
    root = root->tree_insert_value(root, 4);
    root = root->tree_insert_value(root, 7);
    root = root->tree_insert_value(root, 6);
    root = root->tree_insert_value(root, 15);
    root = root->tree_insert_value(root, 19);
    root = root->tree_insert_value(root, 9);
    root = root->tree_insert_value(root, 12);
    root = root->tree_insert_value(root, 13);

    root = root->delete_node_from_binary_search_tree(root, *root->right);
    EXPECT_EQ(root->data, 5);
    EXPECT_EQ(root->left->data, 3);
    EXPECT_EQ(root->right->data, 9);
    EXPECT_EQ(root->right->right->data, 15);
    EXPECT_EQ(root->right->right->left->data, 12);
    EXPECT_EQ(root->right->right->left->right->data, 13);

}