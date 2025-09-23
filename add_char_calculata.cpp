//
// Created by 潘鑫 on 2025/9/15.
//
#include  <gtest/gtest.h>

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

// 确实需要新建立一个这样的符号表，因为确实不一样，因为这里有一个操作，那就是不管前面是什么符号
// 其实这里的内容都相当于添加一个数字到树中，如果之后碰到相对应的括号之后
// 再将之前的括号进行消除
const operate_symbol_and_function bracket_symbols[] = {

};

int get_operate_symbol_priority(char symbol) {
    for (int i = 0; i < sizeof(operate_symbol_and_functions) / sizeof(operate_symbol_and_function); i++) {
        if (operate_symbol_and_functions[i].symbol == symbol) {
            return i / 2;
        }
    }
    for (int i = 0; i < sizeof(bracket_symbols) / sizeof(bracket_symbols); i++) {
        if (bracket_symbols[i].symbol == symbol) {
            return (i + sizeof(operate_symbol_and_functions) / sizeof(operate_symbol_and_function)) / 2;
        }
    }
}


struct tree_node_calculate {
    struct tree_node_calculate *parent;
    struct tree_node_calculate *left;
    struct tree_node_calculate *right;
    int value;
    char calculate_operation;
    char priority;


    int (*operate_function)(int left_value, int right_value);

    int get_operate_symbol_priority() {
        return this->priority;
    }

    struct tree_node_calculate *find_miximum_leaf(struct tree_node_calculate *root) {
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

    struct tree_node_calculate *find_root(struct tree_node_calculate *root) {
        if (root == nullptr) {
            return nullptr;
        } else {
            while (root->parent != nullptr) {
                return find_root(root->parent);
            }
            return root;
        }
    }

    int tree_add_after_node(struct tree_node_calculate *root, struct tree_node_calculate *new_node) {
        if (root == nullptr) {
            root = new_node; // 这里是添加首个数字的位置
        } else {
            struct tree_node_calculate *new_root = root;
            struct tree_node_calculate *miximum_leaf = new_node->find_miximum_leaf(new_root);
            while (miximum_leaf->parent->right != nullptr) {
                miximum_leaf = miximum_leaf->parent;
            }
            add_new_node_to_right(miximum_leaf->parent, new_node);
        }
    }

    int tree_add_operate_to_tree(struct tree_node_calculate *root, struct tree_node_calculate *new_node) {
        if (root == nullptr) {
            root = new_node;
        } else {
            struct tree_node_calculate *new_root = root;
            struct tree_node_calculate *miximum_leaf = new_node->find_miximum_leaf(new_root);
            while (miximum_leaf != new_root->find_root(new_root)) {
                if (miximum_leaf->parent != nullptr) {
                    if (new_node->get_operate_symbol_priority() >
                        miximum_leaf->parent->get_operate_symbol_priority()) {
                        break;
                    }
                    miximum_leaf = miximum_leaf->parent;
                }
            }
            add_new_node_before_parent(miximum_leaf, new_node);
        }
    }

    int add_new_node_to_right(struct tree_node_calculate *node, struct tree_node_calculate *new_node) {
        new_node->right = node->right; // 新节点的右孩子更新为将要原本的右孩子
        node->right = new_node; // 更新node的右孩子
        new_node->parent = node; // 更新新节点的parent
        if (new_node->right != nullptr) {
            new_node->right->parent = new_node; // 如果原本的右孩子不为空，更新原本的父节点
        }
    }

    int add_new_node_before_parent(struct tree_node_calculate *node, struct tree_node_calculate *new_node) {
        new_node->parent = node->parent;
        new_node->left = node;
        node->parent = new_node;
        if (new_node->parent != nullptr && new_node->parent->left == node) {
            new_node->parent->left = new_node;
        } else if (new_node->parent != nullptr && new_node->parent->right == node) {
            new_node->parent->right = new_node;
        }
    }

    int add_node_to_left_child(struct tree_node_calculate *left) {
        this->left = left;
        left->parent = this;
    }

    int add_node_to_right_child(struct tree_node_calculate *right) {
        this->right = right;
        right->parent = this;
    }

    int update_calculate_operate_function(char operator_char) {
        for (int i = 0; i < sizeof(operate_symbol_and_functions) / sizeof(operate_symbol_and_function); i++) {
            if (operate_symbol_and_functions[i].symbol == operator_char) {
                this->operate_function = operate_symbol_and_functions[i].operate_function;
                this->priority = operate_symbol_and_functions[i].priority;
                break;
            }
        }
    }

    int calculate_function(char operator_char) {
    }
};

struct tree_node_calculate *init_a_calculate_note(int value, char calculate_operation) {
    struct tree_node_calculate *new_node = new struct tree_node_calculate;
    if (new_node == nullptr) {
        return nullptr;
    } else {
        new_node->parent = nullptr;
        new_node->left = nullptr;
        new_node->right = nullptr;
        new_node->calculate_operation = calculate_operation;
        new_node->operate_function = nullptr;
        new_node->value = value;
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
        int return_error_value = calculate_subtree(node->left);
        left_value = node->left->value;
    }
    int right_value = 0;
    if (node->right != nullptr) {
        int return_error_value = calculate_subtree(node->right);
        right_value = node->right->value;
    }
    node->value = node->operate_function(left_value, right_value);
}


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
    EXPECT_EQ(root->value, 20);
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

struct number_or_operate {
    int number;
    int operate;
};

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
            int string_length = strlen(argv); // 这行为什么一直有问题？// 原来是前面的for的开始位置有问题
            char *pEnd;
            int li1 = strtol(argv, &pEnd, 0);
            if (*pEnd == ERANGE || !isdigit(argv[0])) {
                // 这里判断是字符的情况
                // 下面一行的循环走不出来。
                for (int j = 0; j < sizeof(operate_symbol_and_functions) / sizeof(operate_symbol_and_function); j++) {
                    if (operate_symbol_and_functions[j].symbol == argv[0]) {
                        struct tree_node_calculate *new_node =
                                init_a_calculate_note(0, operate_symbol_and_functions[j].symbol);
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
                if (new_node->operate_function != nullptr &&
                    (new_node->left == nullptr || new_node->right == nullptr)) {
                    root->tree_add_operate_to_tree(root, new_node);
                } else {
                    root->tree_add_after_node(root, new_node);
                }
                root = new_node->find_root(new_node);
            }
        }
        return root;
    }
}

struct tree_node_calculate *construction_calulate_tree(std::vector<number_or_operate> *number_or_operate_vector_list) {
    if (number_or_operate_vector_list->size() == 0) {
        return nullptr;
    } else {
        struct tree_node_calculate *root = nullptr;
        for (auto number_or_operate_vector: *number_or_operate_vector_list) {
            struct tree_node_calculate *new_node = create_a_node_to_tree(number_or_operate_vector);
            if (new_node != nullptr) {
                if (new_node->operate_function != nullptr &&
                    (new_node->left == nullptr || new_node->right == nullptr)) {
                    root->tree_add_operate_to_tree(root, new_node);
                } else {
                    root->tree_add_after_node(root, new_node);
                }
                root = new_node->find_root(new_node);
            }
        }
        return root;
    }
}


struct tree_node_calculate *construction_calulate_sub_tree(
    std::vector<number_or_operate> *number_or_operate_vector) {
    if (number_or_operate_vector->size() == 0) {
        return nullptr;
    } else {
        struct tree_node_calculate *root = nullptr;
        for (auto number_or_operate_vector: *number_or_operate_vector) {
            struct tree_node_calculate *new_node = create_a_node_to_tree(number_or_operate_vector);
            if (new_node != nullptr) {
                if (new_node->operate_function != nullptr &&
                    (new_node->left == nullptr || new_node->right == nullptr)) {
                    root->tree_add_operate_to_tree(root, new_node);
                } else {
                    root->tree_add_after_node(root, new_node);
                }
                root = new_node->find_root(new_node);
            }
        }
        if (number_or_operate_vector->size() > 1 && root != nullptr) {
            root->priority = 5;
        }
        return root;
    }
}

struct tree_node_calculate *construction_calulate_tree(
    std::vector<std::vector<number_or_operate> *> number_or_operate_vector_list) {
    if (number_or_operate_vector_list.size() == 0) {
        return nullptr;
    } else {
        struct tree_node_calculate *root = nullptr;
        for (auto number_or_operate_vector: number_or_operate_vector_list) {
            struct tree_node_calculate *new_node = construction_calulate_sub_tree(number_or_operate_vector);
            if (new_node != nullptr) {
                if (new_node->operate_function != nullptr &&
                    (new_node->left == nullptr || new_node->right == nullptr)) {
                    root->tree_add_operate_to_tree(root, new_node);
                } else {
                    root->tree_add_after_node(root, new_node);
                }
                root = new_node->find_root(new_node);
            }
        }
        return root;
    }
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
    int string_number = sizeof(calculate_expreesion) / sizeof(calculate_expreesion[0]);
    struct tree_node_calculate *tem = construction_calulate_tree(string_number,
                                                                 static_cast<char **>(calculate_expreesion));
    struct tree_node_calculate *root = tem->find_root(tem);
    calculate_subtree(root);
    EXPECT_EQ(root->value, 80);
}

TEST(calculate, string_array_about_number_2) {
    char *calculate_expreesion[] = {
        "30", "+", "50", "-", "20"
    };
    int string_number = sizeof(calculate_expreesion) / sizeof(calculate_expreesion[0]);
    struct tree_node_calculate *tem = construction_calulate_tree(string_number,
                                                                 static_cast<char **>(calculate_expreesion));
    struct tree_node_calculate *root = tem->find_root(tem);
    calculate_subtree(root);
    EXPECT_EQ(root->value, 60);
}

TEST(calculate, string_array_about_number_3) {
    char *calculate_expreesion[] = {
        "30", "+", "50", "/", "20"
    };

    int string_number = sizeof(calculate_expreesion) / sizeof(calculate_expreesion[0]);
    struct tree_node_calculate *tem = construction_calulate_tree(string_number,
                                                                 static_cast<char **>(calculate_expreesion));
    struct tree_node_calculate *root = tem->find_root(tem);
    calculate_subtree(root);
    EXPECT_EQ(root->value, 32);
}

TEST(calculate, string_array_about_number_4) {
    char *calculate_expreesion[] = {
        "30", "+", "50", "/", "20", "-", "1"
    };

    int string_number = sizeof(calculate_expreesion) / sizeof(calculate_expreesion[0]);
    struct tree_node_calculate *tem = construction_calulate_tree(string_number,
                                                                 static_cast<char **>(calculate_expreesion));
    struct tree_node_calculate *root = tem->find_root(tem);
    calculate_subtree(root);
    EXPECT_EQ(root->value, 31);
}

TEST(calculate, string_array_about_number_5) {
    char *calculate_expreesion[] = {
        "30", "+", "50", "/", "20", "*", "4"
    };

    int string_number = sizeof(calculate_expreesion) / sizeof(calculate_expreesion[0]);
    struct tree_node_calculate *tem = construction_calulate_tree(string_number,
                                                                 static_cast<char **>(calculate_expreesion));
    struct tree_node_calculate *root = tem->find_root(tem);
    calculate_subtree(root);
    EXPECT_EQ(root->value, 38);
}

TEST(calculate, string_array_about_number_6) {
    char *calculate_expreesion[] = {
        "30", "+", "50", "/", "20", "-", "1", "*", "2"
    };

    int string_number = sizeof(calculate_expreesion) / sizeof(calculate_expreesion[0]);
    struct tree_node_calculate *tem = construction_calulate_tree(string_number,
                                                                 static_cast<char **>(calculate_expreesion));
    struct tree_node_calculate *root = tem->find_root(tem);
    calculate_subtree(root);
    EXPECT_EQ(root->value, 30);
}

TEST(calculate, string_array_about_number_7) {
    char *calculate_expreesion[] = {
        "30", "+", "50", "/", "20", "-", "1", "*", "2", "+", "3", "*", "4"
    };

    int string_number = sizeof(calculate_expreesion) / sizeof(calculate_expreesion[0]);
    struct tree_node_calculate *tem = construction_calulate_tree(string_number,
                                                                 static_cast<char **>(calculate_expreesion));
    struct tree_node_calculate *root = tem->find_root(tem);
    calculate_subtree(root);
    EXPECT_EQ(root->value, 42);
}

// 如果添加一个带括号的测试，那么应该怎么算呢？如果添加了一个不同的符号，比如log，应该怎么算呢？
// 先说括号的部分，应该是有两个解决办法的，第一个是在括号出现的时候就去分词，将全部的内容以括号为标准，分为一个小段
// 当需要链接括号部分的时候，就去计算括号部分的链接位置
// 另一个办法是什么呢？将括号也作为一个分割符，如果遇到左括号就添加，如果遇到右括号，就和之前的左括号进行抵消？
// 这里的括号确实很难解，搞不定，需要去走第二条路了

//
TEST(calculate, string_array_about_number_8) {
    char *calculate_expreesion[] = {
        "(", "30", "+", "50", ")", "/", "20", "-", "1", "*", "2", "+", "3", "*", "4"
    };
    char *calculate_string = {
        "(30+50)/20-1*2+3*4"
    };
    std::vector<std::vector<number_or_operate> *> number_or_operate_vector_list = {}; {
        std::vector<number_or_operate> *number_or_operate_vector = new std::vector<number_or_operate>; {
            struct number_or_operate tem = {30, 0};
            number_or_operate_vector->push_back(tem);
        } {
            struct number_or_operate tem = {0, '+'};
            number_or_operate_vector->push_back(tem);
        } {
            struct number_or_operate tem = {50, 0};
            number_or_operate_vector->push_back(tem);
        }
        number_or_operate_vector_list.push_back(number_or_operate_vector); // 明白是怎么回事了，有临时变量，临时变量被清除了
    } {
        std::vector<number_or_operate> *number_or_operate_vector2 = new std::vector<number_or_operate>;
        struct number_or_operate tem = {0, '/'};
        number_or_operate_vector2->push_back(tem);

        number_or_operate_vector_list.push_back(number_or_operate_vector2);
    } {
        std::vector<number_or_operate> *number_or_operate_vector2 = new std::vector<number_or_operate>;
        struct number_or_operate tem = {20, 0};
        number_or_operate_vector2->push_back(tem);

        number_or_operate_vector_list.push_back(number_or_operate_vector2);
    }
    struct tree_node_calculate *tem = construction_calulate_tree(number_or_operate_vector_list);
    struct tree_node_calculate *root = tem->find_root(tem);
    calculate_subtree(root);
    EXPECT_EQ(root->value, 4);

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


const char bracket_symbol[] = {
    {'{'},
    {'}'},
    {'['},
    {']'},
    {'('},
    {')'},
};

int get_if_bracket_symbol(char input_char) {
    for (int i = 0; i < sizeof(bracket_symbol) / sizeof(char); i++) {
        if (bracket_symbol[i] == input_char) {
            return i;
        }
    }
    return -1;
}

struct bracket_symbol_position {
    int index;
    int position_in_string;
    int length;
};

TEST(bracket, string) {
    char *calculate_string = {
        "(80+(30+50)/20)-1*2+3*4"
    };
    int str_length = strlen(calculate_string);
    std::vector<bracket_symbol_position> bracket_symbol_positions = {};
    for (int i = 0; i < str_length; i++) {
        int reslut = get_if_bracket_symbol(calculate_string[i]);
        if (reslut >= 0) {
            bracket_symbol_position auto_para = {};
            auto_para.index = reslut;
            auto_para.position_in_string = i;
            bracket_symbol_positions.push_back(auto_para);
        }
    }
    // 有一个操作是找到上一个为空的，这个操作比较重要
    // 先不管那些内容，先说这个操作应该分为几步？
    // 第一步只管符号的识别，以及确定位置
    // 第二步，已经拿到这个vector了，那么就可以开始考虑匹配括号的事情了
    // 匹配的括号需要添加长度这个操作
    bracket_symbol_position *last_one = nullptr;
    std::stack<bracket_symbol_position *> stack;

    for (int i = 0; i < bracket_symbol_positions.size(); i++) {
        // 这里是需要实现一个栈的，每次进入的时候将指针压栈，然后比较新加入的和上一个是否相同，
        // 相同就把这两个同时出栈
        stack.push(&bracket_symbol_positions[i]);
        bracket_symbol_position *new_one = stack.top();
        if (last_one != nullptr && new_one != nullptr) {
            if (last_one->index % 2 == 0 && (new_one->index - 1) == last_one->index) {
                // 括号匹配的情况
                last_one->length = new_one->position_in_string - last_one->position_in_string;
                new_one->length = last_one->position_in_string - new_one->position_in_string;

                stack.pop();
                stack.pop();

                if (!stack.empty())
                    new_one = stack.top();
            }
        }
        last_one = new_one;
    }
    EXPECT_EQ(0, stack.size());

    // 匹配完长度呢？之后需要做什么呢？
    // 之后需要根据vector中内容创建一个树状的结构，
    // 树状的结构在上面有描述，但是两个树的结构是不同的，
    // 不，其实是这里打算建立的树的结构不合理。合理不合理先做吧，我觉得应该是能够合理起来的。
    //         +
    //    80              /
    //              +          20
    //           30   50
}
