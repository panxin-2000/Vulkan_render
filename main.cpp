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

struct Node {
    struct Node *next_node;
    int value;
};
struct Head {
    struct Node *node;
};


/**
 * 插入的时候应怎么插入呢？ head 的值 指向插入的值，之前的head 被替换为新插入的值
 * @param head
 * @param value
 */
struct Node *insert_value(struct Head *head, int value) {
    struct Node *node = (struct Node *) malloc(sizeof(struct Node));
    node->next_node = head->node;
    node->value = value;
    head->node = node;
    return node;
}

/**
 * 如何删除呢？传入节点的指针，然后开查找，判断是否为空，不为空判断是否和传入的指针相等，如果找到后应该怎么办
 * @param head
 * @param node
 * @return
 */
void delete_node(struct Head *head, struct Node *node) {
    if (head->node != nullptr && head->node == node) {
        head->node = node->next_node;
        std::cout << "已删除对应节点" << std::endl;
        return;
    }
    struct Node *sentinel_node = head->node;
    while (sentinel_node != nullptr) {
        if (sentinel_node->next_node == node) {
            sentinel_node->next_node = node->next_node;
            std::cout << "已删除对应节点" << std::endl;
            return;
        }
        sentinel_node = sentinel_node->next_node;
    }
}

void print_list(struct Head *head) {
    if (head->node == nullptr) {
        std::cout << "链表为空" << std::endl;
        return;
    }
    struct Node *node = head->node;
    std::cout << "node value : " << node->value << std::endl;
    while (node->next_node != nullptr) {
        std::cout << "node value : " << node->next_node->value << std::endl;
        node = node->next_node;
    }
    return;
}

struct Node *find_value(struct Head *head, int value) {
    struct Node *node = head->node;
    while (node != nullptr && node->value != value) {// 当节点不是空的时候才可以检索其值
        node = node->next_node;
    }
    return node; // 这里有一个问题就是找不到的时候会返回最后一个值
    // 刚刚有个导致程序退出的错误，这个错误并不是这里因为的，因为返回了nullptr引起的
}


int main(int argc, char **argv) {
    struct Head *head = (struct Head *) malloc(sizeof(struct Head));
    head->node = nullptr;
    print_list(head);
    insert_value(head, 10);
    insert_value(head, 2);
    insert_value(head, 3);
    insert_value(head, 4);
    struct Node *tem = insert_value(head, 5);
    print_list(head);
    delete_node(head, tem);
    delete_node(head, tem);
    print_list(head);
    tem = find_value(head, 100);
    if (tem != nullptr)
        std::cout << "tem value : " << tem->value << std::endl;
    tem = find_value(head, 2);
    if (tem != nullptr)
        std::cout << "tem value : " << tem->value << std::endl;


//    free(head);



}
