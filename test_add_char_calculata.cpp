//
// Created by 潘鑫 on 2025/9/24.
//
#include  <gtest/gtest.h>
#include "add_char_calculata.h"

TEST(calculate, BasicAssertions) {
    EXPECT_STRNE("hello", "world");

    struct tree_node_calculate *root = init_a_calculate_note(0, '*');
    struct tree_node_calculate *new_node = init_a_calculate_note(2, 0);
    struct tree_node_calculate *new_node_right = init_a_calculate_note(0, '+');
    root->add_node_to_left_child(new_node);
    root->add_node_to_right_child(new_node_right); {
        struct tree_node_calculate *new_node_1 = init_a_calculate_note(5, 0);
        struct tree_node_calculate *new_node_right_2 = init_a_calculate_note(5, 0);
        new_node_right->add_node_to_left_child(new_node_1);
        new_node_right->add_node_to_right_child(new_node_right_2);
    }

    calculate_subtree(root); // 这个测试的主要作用是测试计算的结果对不对，
    // 忽然想起来还能做另外一个测试，那就是只更改其中一个值，然后lazy更新，得出结果

    // 2 * (5 + 5)
    //      *
    // 2        (+)
    //        5     5
    EXPECT_EQ(root->data.value, 20);
}

TEST(tree, tree_root) {
    struct tree_node_calculate *new_node =
            init_a_calculate_note(30, 0);
    struct tree_node_calculate *new_node_2 =
            init_a_calculate_note(0, '+');
    new_node->add_new_node_before_parent(new_node, new_node_2);
    EXPECT_EQ(new_node_2, new_node->find_root(new_node));
}


TEST(calculate, string_array_about_number) {
    char *calculate_expreesion[] = {
        "30", "+", "50"
    };
    int string_number = std::size(calculate_expreesion);
    struct tree_node_calculate *tem = construction_calulate_tree(string_number,
                                                                 static_cast<char **>(calculate_expreesion));
    auto *root = reinterpret_cast<tree_node_calculate *>(tem->find_root(tem));
    calculate_subtree(root);
    EXPECT_EQ(root->data.value, 80);
}

TEST(calculate, string_array_about_number_2) {
    char *calculate_expreesion[] = {
        "30", "+", "50", "-", "20"
    };
    int string_number = sizeof(calculate_expreesion) / sizeof(calculate_expreesion[0]);
    struct tree_node_calculate *tem = construction_calulate_tree(string_number,
                                                                 static_cast<char **>(calculate_expreesion));
    auto *root = static_cast<tree_node_calculate *>(tem->find_root(tem));
    calculate_subtree(root);
    EXPECT_EQ(root->data.value, 60);
}

TEST(calculate, string_array_about_number_3) {
    char *calculate_expreesion[] = {
        "30", "+", "50", "/", "20"
    };

    int string_number = sizeof(calculate_expreesion) / sizeof(calculate_expreesion[0]);
    struct tree_node_calculate *tem = construction_calulate_tree(string_number,
                                                                 static_cast<char **>(calculate_expreesion));
    auto *root = static_cast<tree_node_calculate *>(tem->find_root(tem));
    calculate_subtree(root);
    EXPECT_EQ(root->data.value, 32);
}

TEST(calculate, string_array_about_number_4) {
    char *calculate_expreesion[] = {
        "30", "+", "50", "/", "20", "-", "1"
    };

    int string_number = sizeof(calculate_expreesion) / sizeof(calculate_expreesion[0]);
    struct tree_node_calculate *tem = construction_calulate_tree(string_number,
                                                                 static_cast<char **>(calculate_expreesion));
    auto *root = static_cast<tree_node_calculate *>(tem->find_root(tem));
    calculate_subtree(root);
    EXPECT_EQ(root->data.value, 31);
}

TEST(calculate, string_array_about_number_5) {
    char *calculate_expreesion[] = {
        "30", "+", "50", "/", "20", "*", "4"
    };

    int string_number = sizeof(calculate_expreesion) / sizeof(calculate_expreesion[0]);
    struct tree_node_calculate *tem = construction_calulate_tree(string_number,
                                                                 static_cast<char **>(calculate_expreesion));
    auto *root = static_cast<tree_node_calculate *>(tem->find_root(tem));
    calculate_subtree(root);
    EXPECT_EQ(root->data.value, 38);
}

TEST(calculate, string_array_about_number_6) {
    char *calculate_expreesion[] = {
        "30", "+", "50", "/", "20", "-", "1", "*", "2"
    };

    int string_number = sizeof(calculate_expreesion) / sizeof(calculate_expreesion[0]);
    struct tree_node_calculate *tem = construction_calulate_tree(string_number,
                                                                 static_cast<char **>(calculate_expreesion));
    auto *root = static_cast<tree_node_calculate *>(tem->find_root(tem));
    calculate_subtree(root);
    EXPECT_EQ(root->data.value, 30);
}

TEST(calculate, string_array_about_number_7) {
    char *calculate_expreesion[] = {
        "30", "+", "50", "/", "20", "-", "1", "*", "2", "+", "3", "*", "4"
    };

    int string_number = sizeof(calculate_expreesion) / sizeof(calculate_expreesion[0]);
    struct tree_node_calculate *tem = construction_calulate_tree(string_number,
                                                                 static_cast<char **>(calculate_expreesion));
    auto *root = static_cast<tree_node_calculate *>(tem->find_root(tem));
    calculate_subtree(root);
    EXPECT_EQ(root->data.value, 42);
}

// 如果添加一个带括号的测试，那么应该怎么算呢？如果添加了一个不同的符号，比如log，应该怎么算呢？
// 先说括号的部分，应该是有两个解决办法的，第一个是在括号出现的时候就去分词，将全部的内容以括号为标准，分为一个小段
// 当需要链接括号部分的时候，就去计算括号部分的链接位置
// 另一个办法是什么呢？将括号也作为一个分割符，如果遇到左括号就添加，如果遇到右括号，就和之前的左括号进行抵消？
// 这里的括号确实很难解，搞不定，需要去走第二条路了

//
TEST(calculate, string_array_about_number_8) {
    GTEST_SKIP() << "Skipping single test";
    char *calculate_expreesion[] = {
        "(", "30", "+", "50", ")", "/", "20", "-", "1", "*", "2", "+", "3", "*", "4"
    };
    char *calculate_string = {
        "(30+50)/20-1*2+3*4"
    };
    std::vector<number_or_operate> number_or_operate_vector_list = {}; {
        struct number_or_operate tem = {0, 0, "30+50"};
        number_or_operate_vector_list.push_back(tem); // 明白是怎么回事了，有临时变量，临时变量被清除了
    } {
        struct number_or_operate tem = {0, '/'};
        number_or_operate_vector_list.push_back(tem);
    } {
        struct number_or_operate tem = {20, 0};
        number_or_operate_vector_list.push_back(tem);
    }
    struct tree_node_calculate *tem = construction_calulate_tree(number_or_operate_vector_list);
    auto *root = static_cast<tree_node_calculate *>(tem->find_root(tem));
    calculate_subtree(root);
    EXPECT_EQ(root->data.value, 4);

    // 先不着急分词，先将之后的结果计算好

    // 那么这里就需要首先将括号的部分做为一个完整的表达式了
    // 括号的部分算什么呢？算是一个数值，需要返回它的根结点
    // 在这里就进行处理呢？还是在之前就进行处理呢？
    // 是的，在之前就需要进行处理。
}

// 括号的嵌套想要处理的话，那么首先需要判断括号是否是正确的，之后拿到每个括号的的开始位置和长度长度是由其后括号决定的
// 主要的内容是方案，确定好方案之后，后续的内容其实怎么做都方便
// 识别括号是需要先做的，问题是之后是不括号的数据结构应该是什么样子的？
// 比如我第一个括号里面可能有两个或三个括号，每个括号里面又可能再镶嵌了两套括号
// 那就需要这样表示，左孩子表示下一层的括号，右孩子表示兄弟，如果右孩子还有右孩子，表示有三个括号，依次向下推
// 分为两步，第一个只是检查括号是否正确，并且输出每个括号的开始位置和范围
// 第二次拿到开始位置和范围以及括号的类型之后，再进行相关的树的计算
// 再之后就是确定括号中间的那些是需要进行分词操作的，那些是可以由一个子过程来进行完成
