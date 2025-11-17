//
// Created by 潘鑫 on 2025/9/24.
//
#include "tree_node.h"
#include <gtest/gtest.h>

#include "binary_Tree_Node.h"
#include "index_binary_tree_node.h"
#include "RB_tree_node.h"
#include "tree_function.h"


// 如果只是二叉搜索树，那么需要测试这个，
// 其实应该还可以再加上中序输出的，因为它比较好测试
template<typename T>
void level_tree_walk_test(T node) {
    return level_tree_walk_test(node, static_cast<T>(nullptr), [](T insert_node) { return insert_node; });
}

template<typename T, typename function>
void level_tree_walk_test(T node, T nil_ptr_or_index, function get_node) {
    std::queue<T> tem;
    if (node != nil_ptr_or_index) {
        tem.push(node);
    }
    while (!tem.empty()) {
        T node_tem = tem.front();
        if (get_node(node_tem)->left != nil_ptr_or_index) {
            tem.push(get_node(node_tem)->left);
            EXPECT_LT(get_node(get_node(node_tem)->left)->data, get_node(node_tem)->data);
        }
        if (get_node(node_tem)->right != nil_ptr_or_index) {
            tem.push(get_node(node_tem)->right);
            EXPECT_GE(get_node(get_node(node_tem)->right)->data, get_node(node_tem)->data);
        }
        tem.pop();
    }
}


std::vector<int> first{5, 3, 2, 4, 7, 8};

binary_Tree_Node<int> *init_tree(std::vector<int> vs) {
    binary_Tree_Node<int> *root = nullptr;
    for (auto v: vs) {
        root = root->tree_insert_value(root, v);
    }
    return root;
}

binary_Tree_Node<int> *init_tree() {
    return init_tree(first);
}

TEST(test_tree, binary_Tree_Node_insert) {
    EXPECT_LT(1, 2);
    EXPECT_GE(2, 2);
    auto root = init_tree();
    level_tree_walk_test(root);
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
    // root->right->right->data = 1;
    // level_tree_walk_test(root);
}

template<typename T>
void test_two_vector_value_eq(T left, T right) {
    ASSERT_EQ(left.size(), right.size());
    for (int i = 0; i < left.size(); i++) {
        EXPECT_EQ(left.at(i), right.at(i));
    }
}

template<typename T, typename T1>
void test_two_vector_value_eq_data(T1 left, T right) {
    ASSERT_EQ(left.size(), right.size());
    for (int i = 0; i < left.size(); i++) {
        EXPECT_EQ(left.at(i)->data, right.at(i)->data);
    }
}


template<typename T>
std::vector<T> *init_null_vector(T root) {
    return new std::vector<T>;
}

TEST(test_tree, inorder_tree_walk) {
    auto root = init_tree();
    auto inorder_walk = init_null_vector(root);
    inorder_tree_walk(root, inorder_walk);
    auto temp = inorder_tree_walk_with_stack(root, init_null_vector(root));

    test_two_vector_value_eq(*inorder_walk,
                             *temp);
}

TEST(test_tree, postorder_tree_walk_with_stack) {
    auto root = init_tree();
    auto inorder_walk = init_null_vector(root);
    postorder_tree_walk(root, inorder_walk);
    auto temp = postorder_tree_walk_with_stack(root, init_null_vector(root));

    test_two_vector_value_eq(*inorder_walk,
                             *temp);
}

TEST(test_tree, preorder_tree_walk_with_stack) {
    auto root = init_tree();
    auto inorder_walk = init_null_vector(root);
    preorder_tree_walk(root, inorder_walk);
    auto preorder_walk = preorder_tree_walk_with_stack(root, init_null_vector(root));

    test_two_vector_value_eq(*inorder_walk,
                             *preorder_walk);
}

//       5
//   3      7
// 2   4      8
TEST(test_tree, binary_Tree_Node_delete) {
    binary_Tree_Node<int> *root = nullptr;
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
    preorder_tree_walk_find_father(root, root->right->right);

    // root = root->delete_node_from_binary_search_tree(root, *root->right->right);
    // EXPECT_EQ(root->right->right, nullptr);
}

// 想做一个试试，但是最后发现是一个稍微有点大的大工程
//       5
//   3      7
// 2   4      8
TEST(test_tree, binary_Tree_Node_history) {
    GTEST_SKIP();
    binary_Tree_Node<int> *root = nullptr;
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

    root = root->delete_node_from_binary_search_tree(root, root->right->right);
    EXPECT_EQ(root->right->right, nullptr);
}

//       5
//   3      7
// 2   4      8

//       7
//   3      8
// 2   4
TEST(test_tree, test_tree_delete_root) {
    auto root = init_tree();
    root = root->delete_node_from_binary_search_tree(root, root);
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

    root = root->delete_node_from_binary_search_tree(root, root->right);
    EXPECT_EQ(root->data, 5);
    EXPECT_EQ(root->left->data, 3);
    EXPECT_EQ(root->right->data, 9);
}

//       5
//   3       7
// 2   4   6   10
//            9   11

template<typename T>
T init_tree(T tree, std::vector<int> vs) {
    for (auto v: vs) {
        tree.add_new_node(v);
    }
    return tree;
}


TEST(test_tree, test_tree_delete_index) {
    index_binary_Tree_Node<int, index_Tree_Node_with_father<int> > tree;

    tree = init_tree(tree, first);
    auto vs = new std::vector<index_Tree_Node_with_father<int> >;
    auto reslut = tree.preorder_tree_walk_index();
    auto value_ptr = tree.translate(*reslut);

    auto root = init_tree(first);
    auto inorder_walk = init_null_vector(root);
    preorder_tree_walk(root, inorder_walk);


    test_two_vector_value_eq_data(*value_ptr, *inorder_walk);

    tree.delete_node_from_binary_search_tree(tree.get_root_index());
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


    root = root->delete_node_from_binary_search_tree(root, root->right);
    EXPECT_EQ(root->data, 5);
    EXPECT_EQ(root->left->data, 3);
    EXPECT_EQ(root->right->data, 9);
    EXPECT_EQ(root->right->right->data, 15);
    EXPECT_EQ(root->right->right->left->data, 12);
    EXPECT_EQ(root->right->right->left->right->data, 13);
}


template<typename T, typename T2>
T test_inorder_function(T root, T2 v) {
    for (auto v1: v) {
        root = root->tree_insert_value(root, v1);
    }

    auto inorder_walk = init_null_vector(root);
    inorder_tree_walk_with_stack(root, inorder_walk);
    auto temps = root->translate(*inorder_walk);

    T2 copy_v;
    std::copy(v.begin(), v.end(), std::back_inserter(copy_v));
    std::sort(copy_v.begin(), copy_v.end());

    test_two_vector_value_eq(*temps, copy_v);
    return root;
}

void RB_tree_test_delete_node(std::vector<int> vt, int delete_value) {
    RB_Tree_Node<int> *root = nullptr;
    for (auto v1: vt) {
        root = root->tree_insert_value(root, v1);
    }
    auto temp = tree_find_value(root, delete_value);
    root = root->delete_node_from_binary_search_tree(root, temp);

    auto inorder_walk = init_null_vector(root);
    inorder_tree_walk_with_stack(root, inorder_walk);
    auto temps = root->translate(*inorder_walk);

    std::vector<int> copy_v;
    std::copy(vt.begin(), vt.end(), std::back_inserter(copy_v));
    for (std::vector<int>::iterator it = copy_v.begin(); it != copy_v.end(); ++it) {
        if (*it == delete_value) {
            copy_v.erase(it);
            break;
        }
    }
    std::sort(copy_v.begin(), copy_v.end());

    test_two_vector_value_eq(*temps, copy_v);
    // return root;
}


TEST(test_tree, RB_Tree_Node_insert) {
    RB_Tree_Node<int> *root = nullptr;
    RB_Tree_Node<int> *root1 = nullptr;
    //最先考虑的应该是插入,但是过了5个小时才开始看到插入
    std::vector<int> v = {7, 2, 11, 1, 5, 8, 14, 4, 15};
    std::vector<int> vt = {26, 17, 41, 14, 21, 30, 47, 10, 16, 19, 23, 28, 38, 7, 12, 15, 20, 35, 39, 3};
    root = test_inorder_function(root, vt);
    root1 = test_inorder_function(root1, v);

    // for (auto v1: vt) {
    //     auto temp = tree_find_value(root, v1);
    //     root = root->delete_node_from_binary_search_tree(root, temp);
    // }
    // RB_tree_test_delete_node(vt, 21);
    // RB_tree_test_delete_node(vt, 47);
    // RB_tree_test_delete_node(vt, 12);

    for (auto v3: vt) {
        RB_tree_test_delete_node(vt, v3);
    }
    // 上面的内容是为了测试，全部的点，每个都删除一次，看看有什么问题没有


    // 其实可以想办法写一个层序输出的结果，与最开始的值进行比较
    // 插入时应该是可以随机打乱顺序的
}
