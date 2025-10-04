//
// Created by 潘鑫 on 2025/9/24.
//

#ifndef ADD_CHAR_CALCULATA_H
#define ADD_CHAR_CALCULATA_H
#include "tree_node.h"
#include <vector>

int add_function(int left_value, int right_value);

int reduce_function(int left_value, int right_value);

int multiply_function(int left_value, int right_value);

int divide_function(int left_value, int right_value);

struct operate_symbol_and_function {
    char symbol;
    char priority;

    int (*operate_function)(int left_value, int right_value);
};


const operate_symbol_and_function operate_symbol_and_functions[] = {
    {'+', 1, add_function},
    {'-', 1, reduce_function},
    {'*', 2, multiply_function},
    {'/', 2, divide_function},
    {'{', 3, nullptr},
    {'}', 3, nullptr},
    {'[', 4, nullptr},
    {']', 4, nullptr},
    {'(', 5, nullptr},
    {')', 5, nullptr},
    // 新增下面之后有了一个新的问题，那就是功能不再单一的问题，如果正确的话，是不会执行到后面的
};

int get_operate_symbol_priority(char symbol);


struct tree_node_calculate_date {
    int value;
    char calculate_operation;
    char priority;

    int (*operate_function)(int left_value, int right_value);

    int get_operate_symbol_priority() {
        return this->priority;
    }
};



// using tree_node_calculate_alais = RB_Tree_Node<tree_node_calculate_date>;

class tree_node_calculate : public RB_Tree_Node<tree_node_calculate_date> {
public:
    int tree_add_operate_to_tree(struct tree_node_calculate *root, struct tree_node_calculate *new_node) {
        if (root == nullptr) {
            root = new_node;
        } else {
            struct tree_node_calculate *new_root = root;
            auto miximum_leaf =
                    (new_node->find_miximum_leaf(new_root));
            while (miximum_leaf != new_root->find_root(new_root)) {
                if (miximum_leaf->parent != nullptr) {
                    if (new_node->data.get_operate_symbol_priority() >
                        miximum_leaf->parent->data.get_operate_symbol_priority()) {
                        break;
                    }
                    miximum_leaf = static_cast<tree_node_calculate *>(miximum_leaf->parent);
                }
            }
            add_new_node_before_parent(miximum_leaf, new_node);
        }
    }


    int update_calculate_operate_function(tree_node_calculate *node, char operator_char) {
        for (int i = 0; i < sizeof(operate_symbol_and_functions) / sizeof(operate_symbol_and_function); i++) {
            if (operate_symbol_and_functions[i].symbol == operator_char) {
                node->data.operate_function = operate_symbol_and_functions[i].operate_function;
                node->data.priority = operate_symbol_and_functions[i].priority;
                break;
            }
        }
    }

    int calculate_function(char operator_char) {
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
            update_calculate_operate_function(new_node, calculate_operation);
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

    struct number_or_operate {
        int number;
        int operate;
        char calculate_string[50];
    };

    /**
     *
     * @param argc  只能填1
     * @param argv
     * @return
     */
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
            tree_node_calculate *root = nullptr;
            for (int i = 0; i < argc; i++) {
                tree_node_calculate *new_node = create_a_node_to_tree(1, argv[i]);
                if (new_node != nullptr) {
                    if (new_node->data.operate_function != nullptr &&
                        (new_node->left == nullptr || new_node->right == nullptr)) {
                        tree_add_operate_to_tree(root, new_node);
                    } else {
                        root->tree_add_after_node(root, new_node);
                    }
                    root = static_cast<tree_node_calculate *>(new_node->find_root(new_node));
                }
            }
            return root;
        }
    }


    struct tree_node_calculate *
    construction_calulate_tree(std::vector<number_or_operate> number_or_operate_vector_list) {
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
                        tree_add_operate_to_tree(root, new_node);
                    } else {
                        root->tree_add_after_node(root, new_node);
                    }
                    root = static_cast<tree_node_calculate *>(new_node->find_root(new_node));
                }
            }
            return root;
        }
    }
};
#endif //ADD_CHAR_CALCULATA_H
