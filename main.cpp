#include <iostream>
#include <string>
#include <map>
#include <list>
#include <algorithm>
#include <math.h>
#include <forward_list>
#include <unistd.h>

struct Node {
    int value;
    struct Node *next;
};

struct Head {
    struct Node *next;
};




void up(int n){
    std::cout << "\033["<< n << "A";
}
void down(int n){
    std::cout << "\033["<< n << "B";
}
void right(int n){
    std::cout << "\033["<< n << "C";
}
void left(int n){
    std::cout << "\033["<< n << "D";
}
enum VT_100_color{
    black = 40,
    dark_red,
    green,
    yellow,
    blue,
    purple,
    dark_green,
    white
};
void set_color(enum VT_100_color color){
    std::cout << "\033[0m\033["<< color << "m";
}

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
    std::cout << "\033[2J\033[?25l\033[0;0H";
    for (auto iterator = fl.begin(); iterator != fl.end(); ++iterator) {
        std::cout << *iterator << " ";
    }
    std::cout << std::endl;
    std::cout << "\033[40m 123456 \033[0m";
    std::cout << "\033[41m 123456 \033[0m";
    std::cout << std::endl;
    int a = 0;
    while (1) {
        ++a;
        for (int i = 0; i < 15; ++i) {
            for (int i = 0; i < 30; ++i)
                std::cout << "\033[41m"<<' '<<"\033[0m";
            std::cout << "\n\033[0m";
        }
        std::cout << "\033[15A";
        down(2);
        right(4);
        set_color(black);
        std::cout << "  ";
        up(2);
        left(6);
        std::cout << "\033[2;0H";
        std::cout.flush();
        sleep(1);
    }
}
