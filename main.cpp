#include <iostream>
#include <string>
#include <map>
#include <list>
#include <algorithm>
#include <math.h>
#include <forward_list>
#include <unistd.h>
#include <curses.h>

void exit_function(int sig) {
    endwin();// 刚刚删除的掉的一行没有任何影响
    exit(0);
}

struct Tree_Node {
    struct Tree_Node *parent;
    struct Tree_Node *left;
    struct Tree_Node *right;
    int value;
};

void inorder_tree_walk(struct Tree_Node *node) {
    if (node != nullptr) {
        inorder_tree_walk(node->left);
        std::cout << node->value << std::endl;
        inorder_tree_walk(node->right);
    }
}

void preorder_tree_walk(struct Tree_Node *node) {
    if (node != nullptr) {
        std::cout << node->value << std::endl;
        preorder_tree_walk(node->left);
        preorder_tree_walk(node->right);
    }
}

void postorder_tree_walk(struct Tree_Node *node) {
    if (node != nullptr) {
        postorder_tree_walk(node->left);
        postorder_tree_walk(node->right);
        std::cout << node->value << std::endl;
    }
}

void level_tree_walk(struct Tree_Node *node) {
    if (node != nullptr) {
        std::cout << node->value << std::endl;
        level_tree_walk(node->left);
        level_tree_walk(node->right);
    }
}

/**
 * 递归版本的
 * @param tree_node
 * @param value
 * @return
 */
struct Tree_Node *tree_search(struct Tree_Node *tree_node, int value) {

    if (tree_node == nullptr || tree_node->value == value) {
        return tree_node;
    }
    if (value < tree_node->value) {
        return tree_search(tree_node->left, value);
    }
    return tree_search(tree_node->right, value);
}

/**
 * 迭代版本的，迭代版本的一般来说更加快，不需要向栈复制或者储存内容？（是因为这样吗？临时变量）
 * @param tree_node
 * @param value
 * @return
 */
struct Tree_Node *iterative_tree_search(struct Tree_Node *tree_node, int value) {
    while (tree_node != nullptr && tree_node->value != value) {
        if (value < tree_node->value)
            tree_node = tree_node->left;
        else {
            tree_node = tree_node->right;
        }
    }
    return tree_node;
}

struct Tree_Node *tree_maximum(struct Tree_Node *tree_node) {
    while (tree_node != nullptr) {
        tree_node = tree_node->right;
    }
    return tree_node;
}

struct Tree_Node *tree_minimum(struct Tree_Node *tree_node) {
    while (tree_node != nullptr) {
        tree_node = tree_node->left;
    }
    return tree_node;
}

struct Tree_Node *tree_successor(struct Tree_Node *tree_node) {
    if (tree_node->right != nullptr) {
        return tree_minimum(tree_node->right);
    }
    struct Tree_Node *y = tree_node->parent;
    while (y != nullptr && y->right == tree_node) {
        tree_node = y;
        y = y->parent;
    }
    return tree_node;
}

/**
 * 这个函数大概是这个样子，其中最重要的是搜索树的性能，左边的均不大于，右边的均不小于
 * 重要的是如何插入和删除，因为有特殊的插入和删除，才有搜索树的通用查找算法
 * @param tree_node
 * @return
 */
struct Tree_Node *tree_predecessor(struct Tree_Node *tree_node) {
    if (tree_node->left != nullptr) {
        return tree_minimum(tree_node->left);
    }
    struct Tree_Node *y = tree_node->parent;
    while (y != nullptr && y->left == tree_node) {
        tree_node = y;
        y = y->parent;
    }
    return tree_node;
}


struct Tree_Node *tree_insert(struct Tree_Node *root_node, struct Tree_Node &insert_tree_node) {
    struct Tree_Node *will_insert_node = nullptr;
    struct Tree_Node *if_tem_equal = root_node;
    while (if_tem_equal != nullptr) {
        will_insert_node = if_tem_equal;
        if (insert_tree_node.value < if_tem_equal->value)
            if_tem_equal = if_tem_equal->left;
        else
            if_tem_equal = if_tem_equal->right;
    }
    insert_tree_node.parent = will_insert_node;
    if (will_insert_node == nullptr) {
        root_node = &insert_tree_node;
    } else if (insert_tree_node.value < will_insert_node->value) {
        will_insert_node->left = &insert_tree_node;
    } else {
        will_insert_node->right = &insert_tree_node;
    }
    return root_node;
}

struct Tree_Node *tree_insert_value(struct Tree_Node *root_node, int value) {
    struct Tree_Node &nodes = *new struct Tree_Node;
    nodes.right = nullptr;
    nodes.left = nullptr;
    nodes.parent = nullptr;
    nodes.value = value;
    return tree_insert(root_node, nodes);
}


int main(int argc, char **argv) {
    struct Tree_Node *root = nullptr;
//    struct Tree_Node &root = *rootb; //这里编译时能够编译过的，但是实际上问题很大，引用一定是指向了的实际的内容
//    struct Tree_Node* &root = rootb; //这里编译时能够编译过的实际执行也能通过，但是我不知道自己到底在写什么

    //最先考虑的应该是插入,但是过了5个小时才开始看到插入
    root = tree_insert_value(root, 5);
    root = tree_insert_value(root, 3);
    root = tree_insert_value(root, 2);
    root = tree_insert_value(root, 4);
    root = tree_insert_value(root, 7);
    root = tree_insert_value(root, 8);
    // 这里的问题是形参和实参的问题，
    // 能不能用还需要试试。
    // 引用的意思其实还是传递的指针，只是比较隐蔽。
    // 插入的时候改引用还是比较简单的，
    // 先暂时保存一下，
    // 想让指向跟结点的指针变成引用是不可能的，也就是指针是不能变成引用的？


    //  5
    // 3  7
    //2 4  8
    // 输入的以上这些数，但是最后却多了一个零，为什么？
    inorder_tree_walk(root);

}

void test_array_add() {
    int a[6][6];
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 6; ++j) {
            a[j][i] = 0;
        }
    }
    for (int i = 0; i < 6; ++i) {
        a[i][i] = 1;
    }
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 6 - 1 - i; ++j) {
            a[j][j + i + 1] = a[j][j + i] + a[j + 1][j + i + 1];

            std::cout << "a[" << j << "][" << j + i + 1 << "] = ";
            std::cout << "a[" << j << "][" << j + i << "] + ";
            std::cout << "a[" << j + 1 << "][" << j + i + 1 << "]  " << std::endl;
        }
    }
    for (int i = 0; i < 6; ++i) {
        std::cout << " ";
        for (int j = 0; j < 6; ++j) {
            std::cout << a[i][j] << " ";
        }
        std::cout << std::endl;
    }//二维数组都是先行后列
}