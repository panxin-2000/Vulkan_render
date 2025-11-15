//
// Created by 潘鑫 on 2025/9/24.
//
#include "tree_node.h"
#include <gtest/gtest.h>

#include "binary_Tree_Node.h"
#include "RB_tree_node.h"
#include "tree_function.h"


TEST(test_tree, binary_Tree_Node_insert) {
    binary_Tree_Node<int> *root = nullptr;
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

TEST(test_tree, inorder_tree_walk) {
    binary_Tree_Node<int> *root = nullptr;
    //最先考虑的应该是插入,但是过了5个小时才开始看到插入
    root = root->tree_insert_value(root, 5);
    root = root->tree_insert_value(root, 3);
    root = root->tree_insert_value(root, 2);
    root = root->tree_insert_value(root, 4);
    root = root->tree_insert_value(root, 7);
    root = root->tree_insert_value(root, 8);
    auto inorder_walk = new std::vector<binary_Tree_Node<int> *>;
    inorder_tree_walk(root, inorder_walk);
    auto inorder_walk_stack = new std::vector<binary_Tree_Node<int> *>;
    inorder_tree_walk_with_stack(root, inorder_walk_stack);

    ASSERT_EQ(inorder_walk_stack->size(), inorder_walk->size());
    for (int i = 0; i < inorder_walk_stack->size(); i++) {
        EXPECT_EQ(inorder_walk_stack->at(i), inorder_walk->at(i));
    }
}

TEST(test_tree, postorder_tree_walk_with_stack) {
    binary_Tree_Node<int> *root = nullptr;
    //最先考虑的应该是插入,但是过了5个小时才开始看到插入
    root = root->tree_insert_value(root, 5);
    root = root->tree_insert_value(root, 3);
    root = root->tree_insert_value(root, 2);
    root = root->tree_insert_value(root, 4);
    root = root->tree_insert_value(root, 7);
    root = root->tree_insert_value(root, 8);
    auto inorder_walk = new std::vector<binary_Tree_Node<int> *>;
    postorder_tree_walk(root, inorder_walk);
    auto inorder_walk_stack = new std::vector<binary_Tree_Node<int> *>;
    postorder_tree_walk_with_stack(root, inorder_walk_stack);

    ASSERT_EQ(inorder_walk_stack->size(), inorder_walk->size());
    for (int i = 0; i < inorder_walk_stack->size(); i++) {
        EXPECT_EQ(inorder_walk_stack->at(i), inorder_walk->at(i));
    }
}

//       5
//   3      7
// 2   4      8
TEST(test_tree, binary_Tree_Node_delete) {
    binary_Tree_Node<int> *root = nullptr;
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

// 想做一个试试，但是最后发现是一个稍微有点大的大工程
//       5
//   3      7
// 2   4      8
TEST(test_tree, binary_Tree_Node_history) {
    GTEST_SKIP();
    binary_Tree_Node<int> *root = nullptr;
    //最先考虑的应该是插入,但是过了5个小时才开始看到插入
    root = root->tree_insert_value_with_history(root, 5);
    EXPECT_EQ(root->data, 5);
    root = root->tree_insert_value_with_history(root, 3);
    EXPECT_EQ(root->data, 5);
    EXPECT_EQ(root->left->data, 3);
    root = root->tree_insert_value_with_history(root, 2);
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
    binary_Tree_Node<int> *root = nullptr;
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
    binary_Tree_Node<int> *root = nullptr;
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
    binary_Tree_Node<int> *root = nullptr;
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

    auto new_node = tree_find_value(root, 5);
    EXPECT_EQ(new_node->data, 5);
    EXPECT_EQ(root->left->data, 3);
    EXPECT_EQ(root->right->data, 7);


    root = root->delete_node_from_binary_search_tree(root, *root->right);
    EXPECT_EQ(root->data, 5);
    EXPECT_EQ(root->left->data, 3);
    EXPECT_EQ(root->right->data, 9);
    EXPECT_EQ(root->right->right->data, 15);
    EXPECT_EQ(root->right->right->left->data, 12);
    EXPECT_EQ(root->right->right->left->right->data, 13);
}


TEST(test_tree, RB_Tree_Node_insert) {
    RB_Tree_Node<int> *root = nullptr;
    //最先考虑的应该是插入,但是过了5个小时才开始看到插入
    std::vector<int> v = {7, 2, 11, 1, 5, 8, 14, 4, 15};
    std::vector<int> vt = {26, 17, 41, 14, 21, 30, 47, 10, 16, 19, 23, 28, 38, 7, 12, 15, 20, 35, 39, 3};
    for (auto v1: v) {
        root = root->tree_insert_value(root, v1);
    }
    EXPECT_EQ(root->data, 7);
    EXPECT_EQ(root->left->data, 2);
    EXPECT_EQ(root->right->data, 11);
    EXPECT_EQ(root->left->left->data, 1);
    EXPECT_EQ(root->left->right->data, 5);
    EXPECT_EQ(root->right->left->data, 8);
    EXPECT_EQ(root->right->right->data, 14);
    EXPECT_EQ(root->left->right->left->data, 4);
    EXPECT_EQ(root->right->right->right->data, 15);
    auto temp = tree_find_value(root, 5);
    root = root->delete_node_from_binary_search_tree(root, temp);

    // 其实可以想办法写一个层序输出的结果，与最开始的值进行比较
    // 插入时应该是可以随机打乱顺序的
}
