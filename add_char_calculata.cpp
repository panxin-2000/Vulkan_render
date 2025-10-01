//
// Created by 潘鑫 on 2025/9/15.
//
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "add_char_calculata.h"
// 应该怎么分解这部分的内容

// 首先是输入的部分，需要将内容分解为一个逻辑操作树

//分解为逻辑操作树之后，然后需要使用后序遍历，确实是后序的遍历

//不过层序的顺序却是反向来的，或者说，先左，后右，左右都结束之后再去计算父结点
//三种顺序其实只是中间结点在与左右的顺序相对比，其实左右的顺序也是可以相反的，

// 那么可以去规划每个节点有什么内容了
// 父，左，右，这个三个是不可少的
// 然后是节点本身的值，如果是叶子结点，那么需要读取节点本身的值，如果是其他结点，那么需要存储符号，以及子树计算后的结果

int add_function(int left_value, int right_value) {
    return left_value + right_value;
}

int reduce_function(int left_value, int right_value) {
    return left_value - right_value;
}

int multiply_function(int left_value, int right_value) {
    return left_value * right_value;
}

int divide_function(int left_value, int right_value) {
    return left_value / right_value;
}


int get_operate_symbol_priority(const char symbol) {
    for (int i = 0; i < sizeof(operate_symbol_and_functions) / sizeof(operate_symbol_and_function); i++) {
        if (operate_symbol_and_functions[i].symbol == symbol) {
            return i / 2;
        }
    }
    return -1;
}


struct tree_node_calculate *init_a_calculate_note(int value, char calculate_operation) {
    auto *new_node = new tree_node_calculate;
    // 这里提示有问题应该是和某些构造函数有关
    if (new_node == nullptr) {
        return nullptr;
    } else {
        new_node->parent = nullptr;
        new_node->left = nullptr;
        new_node->right = nullptr;
        new_node->data.calculate_operation = calculate_operation;
        new_node->data.operate_function = nullptr;
        new_node->data.value = value;
        new_node->update_calculate_operate_function(calculate_operation);
        return new_node;
    }
}


/**
 * 这个函数的目的为了将树中列好的数据结构进行计算出结果
 * 并不是为了之后更加优化的生成计算机能够执行的表达式
 * 就不去做进一步计算的优化了
 * @param node
 * @return
 */
int calculate_subtree(struct tree_node_calculate *node) {
    if (node == nullptr) {
        // 添加一个错误打印, 表示在计算什么的出错，需要给出提示
        return -1;
    }
    if (node->left == nullptr && node->right == nullptr) {
        return 0;
    }
    int left_value = 0;
    if (node->left != nullptr) {
        int return_error_value = calculate_subtree(static_cast<tree_node_calculate *>(node->left));
        left_value = node->left->data.value;
    }
    int right_value = 0;
    if (node->right != nullptr) {
        int return_error_value = calculate_subtree(static_cast<tree_node_calculate *>(node->right));
        right_value = node->right->data.value;
    }
    node->data.value = node->data.operate_function(left_value, right_value);
    return 0;
}


// 2 * (5 + 5) + (5 + 5) 应该如何解析呢？解析的分界线是什么？
// 先遍历一遍括号，只将最底层的括号合并
// 再遍历一遍乘法，只将乘法合并
// 再遍历一遍括号，依次循环，最后？？
// 其实应该是有另一个解法的吧，应该是 5  5  +  2  *  5  5 +  +  可能会成为这样的一个式子，按照从栈的方式计算，

/**
 *    2    &        *       &              *            &           *                   &          *
 *         &    2       (   &          2       (        &       2       (               &      2        +
 *         &                &                           &                 +             &             5   5
 *         &                &                 5         &              5     5          &
 * &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
 *                      +   &                     +     &                   +           &                  +
 *          *               &         *              (  &         *              (      &        *               +
 *      2        +          &    2        +        5    &    2        +           +     &   2        +         5    5
 *             5   5        &           5   5           &           5   5       5   5   &          5   5
 *
 *
 */
// 过程应该是这样一个过程，写下来之后就会清楚很多
// 那么就是明天中午，看看能不能搞定, // 已经中午了，发现上面的操作似乎都是在增加后继
// 如果数字增加的计算符号的话，是增它的后继，不过这个后继是放在了父结点
// 然后在增加时，判断最大的是否存在左孩子但是没有右孩子，如果是这中情况，就增加到右孩子
// 再增加数字时，判断最大的有没有左右孩子，有的话就是左或右
// 再增加符号时找到最大的数字(应该可以可以说是最大的叶子结点)，如果是左，将符号设置为其父，
// 如果是右孩子，那就依次寻找其父亲，直到其是其父亲的左孩子或者到达根结点，

// 上面的考虑的都是双目操作符，并没有考虑单目操作符，单目操作符的需要直接置要操作的数字为右孩子
// 看起是能够成功的，
// 如果是符号的话，找到最大的叶子结点，然后判断它是否根结点，是的话更新根结点；是左还是右？只能是右，

// 第二天中午的时候，开始想了，首先发现了一个问题，那就是怎么分割符号，这个事情忘记应该怎么做了
// 如果不考虑分割的话，那么我应该用什么形式来管理输入呢？
// 不考虑分割，那么输入的内容就是 int argc, char **argv
// 2 * (5 + 5) + (5 + 5 + 6)
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &                   +           &                   +             &                   +
// &         *              (      &         *              (        &         *              (
// &    2        +           +     &    2        +                +  &    2        +                +
// &           5   5       5   5   &           5   5         +       &           5   5         +        6
//                                 &                       5   5     &                       5   5   
//

// 2 + 7 * (5 + 5) + (5 + 5 + 6)   左边是不行的，按照之前的说法是不对的，向上更新的时候还需要判断优先级
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &        +       &       +            &       +            &         +              &         +
// &     2     7    &   2        *       &   2        *       &     2        *         &     2        *
// &                &         7     (    &         7        ( &           7        (   &           7        (
// &                &                    &                    &                  5     &                    +
// &                &                    &                    &                        &                  5   5


/**
 * 如果是单个参数输入，那么其左右孩子都为空
 * @param argc
 * @param argv
 * @return
 */
struct tree_node_calculate *create_a_node_to_tree(int argc, char *argv) {
    if (argc == 0) {
        return nullptr;
    } else {
        struct tree_node_calculate *root = nullptr;
        for (int i = 0; i < argc; i++) {
            int string_length = std::strlen(argv); // 这行为什么一直有问题？// 原来是前面的for的开始位置有问题
            char *pEnd;
            const long li1 = std::strtol(argv, &pEnd, 0);
            if (*pEnd == ERANGE || !std::isdigit(argv[0])) {
                // 这里判断是字符的情况
                // 下面一行的循环走不出来。
                for (auto j: operate_symbol_and_functions) {
                    if (j.symbol == argv[0]) {
                        struct tree_node_calculate *new_node =
                                init_a_calculate_note(0, j.symbol);
                        return new_node;
                    }
                }
            } else {
                // 这里再判断是否是数字的情况
                struct tree_node_calculate *new_node =
                        init_a_calculate_note(li1, 0);
                return new_node;
                // 最后如果是非法字符，需要报错，等待更新后重新判断
            }
        }
    }
}

struct tree_node_calculate *create_a_node_to_tree(number_or_operate tem) {
    struct tree_node_calculate *new_node =
            init_a_calculate_note(tem.number, tem.operate);
    return new_node;
}


// 下面这个函数的改动部分稍微有点大
// 主要是为了表达更加简洁
// create_a_node_to_tree不能输出更多的形式
//方便能将将某些内容直接添加到函数
struct tree_node_calculate *construction_calulate_tree(int argc, char **argv) {
    if (argc == 0) {
        return nullptr;
    } else {
        struct tree_node_calculate *root = nullptr;
        for (int i = 0; i < argc; i++) {
            struct tree_node_calculate *new_node = create_a_node_to_tree(1, argv[i]);
            if (new_node != nullptr) {
                if (new_node->data.operate_function != nullptr &&
                    (new_node->left == nullptr || new_node->right == nullptr)) {
                    root->tree_add_operate_to_tree(root, new_node);
                } else {
                    root->tree_add_after_node(root, new_node);
                }
                root = static_cast<tree_node_calculate *>(new_node->find_root(new_node));
            }
        }
        return root;
    }
}

struct tree_node_calculate *construction_calulate_tree(std::vector<number_or_operate> number_or_operate_vector_list) {
    if (number_or_operate_vector_list.empty()) {
        return nullptr;
    } else {
        struct tree_node_calculate *root = nullptr;
        struct tree_node_calculate *new_node;
        for (auto number_or_operate_vector: number_or_operate_vector_list) {
            if (strlen(number_or_operate_vector.calculate_string) != 0) {
                std::vector<number_or_operate> number_or_operate_vector_list_new = {};

                // 还是需要在下面一行进行递归，// 问题是需要进行重新解算以及识别
                new_node = construction_calulate_tree(number_or_operate_vector_list_new);
            } else {
                new_node = create_a_node_to_tree(number_or_operate_vector);
            }
            if (new_node != nullptr) {
                if (new_node->data.operate_function != nullptr) {
                    root->tree_add_operate_to_tree(root, new_node);
                } else {
                    root->tree_add_after_node(root, new_node);
                }
                root = static_cast<tree_node_calculate *>(new_node->find_root(new_node));
            }
        }
        return root;
    }
}
