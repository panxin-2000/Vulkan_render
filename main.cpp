#include <iostream>
#include <string>
#include <map>
#include <list>
#include <algorithm>
#include <math.h>
#include <forward_list>

struct Node {
    int value;
    struct Node *next;
};

struct Head {
    struct Node *next;
};

int main(int argc, char **argv) {
    std::forward_list<int> fl;
    fl.push_front(10);
    fl.push_front(20);
    fl.push_front(30);


    auto iteratoe = fl.insert_after(fl.begin(), 10);
    iteratoe = std::find(fl.begin(), fl.end(), 10);
    // 上面执行的是find操作，也就是==操作符被重载了。
    // 问题是被重载的操作符有时候我根本不知道从哪里看是否被重载了
    // C++标准库中给出了参考代码，但是呢？和真正的代码是不一样的
    // cppreference 中确实给出了相应的参考代码
    fl.insert_after(iteratoe, 50);
    for (auto iterator = fl.begin(); iterator != fl.end(); ++iterator) {
        std::cout << *iterator << " ";
    }
    std::cout << std::endl;



}