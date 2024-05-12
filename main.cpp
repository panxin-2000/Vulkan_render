#include <cassert>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
typedef struct list {
    int value;
    struct list *next;
} List;


void insert(List **head, int value) {
    List **indirect = head;
    List *tem = (list *) malloc(sizeof(*tem));
    tem->value = value;
    tem->next = NULL;

    while (*indirect != NULL) {
        indirect = &((*indirect)->next);
    }
    *indirect = tem;

}

/*
 * 下面的两个函数解决了指针和指向指针的指针的区别
 * 准确的说它们的区别其实一个int值有没有被多指一次
 */
void pp_print_value(List **ptr) {
    while (*ptr != NULL) {
        printf("%d ", (*ptr)->value);
        ptr = &(*ptr)->next;
    }
    printf("\n");
}

void p_print_value(List *ptr) {
    while (ptr != NULL) {
        printf("%d ", ptr->value);
        ptr = ptr->next;
    }
    printf("\n");
}

void remove_list_entry(List **head, List *entry) {
    List **indirect = head;

    while ((*indirect) != entry) {
        indirect = &(*indirect)->next;
    }

    *indirect = entry->next;
}

/*
 * 反转队列
 */

List *invert_list(List **head) {

    List *prev = NULL;
    List *curr = *head;
    while (curr != NULL) {
        List *nextTemp = curr->next;
        curr->next = prev;
        prev = curr;
        curr = nextTemp;
    }
    return prev;
}


int main() {
    int pi;
    pi = 2;
    assert((7<pi));
    char a[16]={"abcdef"};
    printf("%d %d %d\n",sizeof (a),sizeof (*a),sizeof (&a));
    printf("%d %d %d\n",strlen (a),strlen (( char *)&a));


    List *head = NULL;//默认值不是0，而是一个其他的值，所以第一次运行的时候出问题了。
    insert(&head, 1);
    insert(&head, 2);
    insert(&head, 3);
    insert(&head, 4);
    insert(&head, 5);
    insert(&head, 6);


    p_print_value(head);//传递指针时，把申请的指针符号的值(也是一个地址)给参数就可以，传参的过程可以参考第12行的代码
    head= invert_list(&head);
    pp_print_value(&head);//传递双重指针时，把申请的指针符号的地址给参数就可以

    List *rm2 = head->next;
    List *rm4 = head->next->next->next;

    remove_list_entry(&head, head);
    p_print_value(head);

    remove_list_entry(&head, rm2);
    p_print_value(head);

    remove_list_entry(&head, rm4);
    p_print_value(head);


    printf("hello");//不会在执行完这句后就立刻打印出来。
    return 0;
}
