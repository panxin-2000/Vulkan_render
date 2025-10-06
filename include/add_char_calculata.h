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

class tree_node_calculate : public balance_Tree_Node<tree_node_calculate_date> {
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


    int update_calculate_operate_function(char operator_char) {
        for (int i = 0; i < sizeof(operate_symbol_and_functions) / sizeof(operate_symbol_and_function); i++) {
            if (operate_symbol_and_functions[i].symbol == operator_char) {
                this->data.operate_function = operate_symbol_and_functions[i].operate_function;
                this->data.priority = operate_symbol_and_functions[i].priority;
                break;
            }
        }
    }

    int calculate_function(char operator_char) {
    }
};


struct tree_node_calculate *init_a_calculate_note(int value, char calculate_operation);

int calculate_subtree(struct tree_node_calculate *node);


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
struct tree_node_calculate *create_a_node_to_tree(int argc, char *argv);

struct tree_node_calculate *create_a_node_to_tree(number_or_operate tem);

struct tree_node_calculate *construction_calulate_tree(int argc, char **argv);

struct tree_node_calculate *construction_calulate_tree(std::vector<number_or_operate> number_or_operate_vector_list);
#endif //ADD_CHAR_CALCULATA_H
