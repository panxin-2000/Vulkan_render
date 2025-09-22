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

    int (*operate_function)(int left_value, int right_value);
};

const operate_symbol_and_function operate_symbol_and_functions[] = {
    {'+', add_function},
    {'-', reduce_function},
    {'*', multiply_function},
    {'/', divide_function},
    // 新增下面之后有了一个新的问题，那就是功能不再单一的问题，如果正确的话，是不会执行到后面的

    {'(', nullptr},
    {')', nullptr},
    {'[', nullptr},
    {']', nullptr},
    {'{', nullptr},
    {'}', nullptr},
};

int get_operate_symbol_priority(char symbol) {
    for (int i = 0; i < sizeof(operate_symbol_and_functions) / sizeof(operate_symbol_and_function); i++) {
        if (operate_symbol_and_functions[i].symbol == symbol) {
            return i / 2;
        }
    }
}


struct tree_node_calculate {
    struct tree_node_calculate *parent;
    struct tree_node_calculate *left;
    struct tree_node_calculate *right;
    int value;
    char calculate_operation;

    int (*operate_function)(int left_value, int right_value);

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
                    if (get_operate_symbol_priority(new_node->calculate_operation) >
                        get_operate_symbol_priority(miximum_leaf->parent->calculate_operation)) {
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


struct tree_node_calculate *construction_calulate_tree(int argc, char **argv) {
    if (argc == 0) {
        return nullptr;
    } else {
        struct tree_node_calculate *root = nullptr;
        for (int i = 0; i < argc; i++) {
            int string_length = strlen(argv[i]); // 这行为什么一直有问题？// 原来是前面的for的开始位置有问题
            char *pEnd;
            int li1 = strtol(argv[i], &pEnd, 0);
            if (*pEnd == ERANGE || !isdigit(argv[i][0])) {
                // 这里判断是字符的情况
                // 下面一行的循环走不出来。
                for (int j = 0; j < sizeof(operate_symbol_and_functions) / sizeof(operate_symbol_and_function); j++) {
                    if (operate_symbol_and_functions[j].symbol == argv[i][0]) {
                        struct tree_node_calculate *new_node =
                                init_a_calculate_note(0, operate_symbol_and_functions[j].symbol);
                        root->tree_add_operate_to_tree(root, new_node);
                        root = new_node->find_root(new_node);
                        break;
                    }
                }
            } else {
                // 这里再判断是否是数字的情况
                struct tree_node_calculate *new_node =
                        init_a_calculate_note(li1, 0);
                root->tree_add_after_node(root, new_node);
                root = new_node->find_root(new_node);
                // 最后如果是非法字符，需要报错，等待更新后重新判断
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
