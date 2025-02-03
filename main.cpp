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



/**
 * 插入的时候应怎么插入呢？ head 的值 指向插入的值，之前的head 被替换为新插入的值
 * @param head
 * @param value
 */
struct Node *insert_value(struct Node *head, int value) {
    struct Node *node = (struct Node *) malloc(sizeof(struct Node));
    node->next_node = head->next_node;
    node->value = value;
    head->next_node = node;
    return node;
}

/**
 * 如何删除呢？传入节点的指针，然后开查找，判断是否为空，不为空判断是否和传入的指针相等，如果找到后应该怎么办
 * @param head
 * @param node
 * @return
 */
void delete_node(struct Node *head, struct Node *node) {
    struct Node *sentinel_node = head;
    while (sentinel_node != nullptr) {
        if (sentinel_node->next_node == node) {
            sentinel_node->next_node = node->next_node;
            std::cout << "已删除对应节点" << std::endl;
            return;
        }
        sentinel_node = sentinel_node->next_node;
    }
}

void print_list(struct Node *head) {
    struct Node *node = head->next_node;
    while (node != nullptr) {
        std::cout << "node value : " << node->value << std::endl;
        node = node->next_node;
    }
    return;
}

void inverse_list(struct Node *head) {
    //单链表翻转有点难，但是基本的图已经绘制完成了

}

struct Node *find_value(struct Node *head, int value) {
    struct Node *node = head->next_node;
    while (node != nullptr && node->value != value) {// 当节点不是空的时候才可以检索其值
        node = node->next_node;
    }
    return node; // 这里有一个问题就是找不到的时候会返回最后一个值
    // 刚刚有个导致程序退出的错误，这个错误并不是这里因为的，因为返回了nullptr引起的
}


int main(int argc, char **argv) {
    struct Node *head = (struct Node *) malloc(sizeof(struct Node));
    head->next_node = nullptr;
    print_list(head);
    struct Node *tem_1 = insert_value(head, 1);
    struct Node *tem_2 = insert_value(head, 2);
    struct Node *tem_3 = insert_value(head, 3);
    struct Node *tem_4 = insert_value(head, 4);
    struct Node *tem_5 = insert_value(head, 5);
    print_list(head);

    delete_node(head, tem_5);
    delete_node(head, tem_1);
    delete_node(head, tem_3);
    delete_node(head, tem_5);
    delete_node(head, tem_1);
    delete_node(head, tem_3);
    print_list(head);
    struct Node * tem = find_value(head, 100);
    if (tem != nullptr)
        std::cout << "tem value : " << tem->value << std::endl;
    tem = find_value(head, 2);
    if (tem != nullptr)
        std::cout << "tem value : " << tem->value << std::endl;


//    free(head);



}
