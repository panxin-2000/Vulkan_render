#include <iostream>
#include <string>
#include <map>
#include <list>
#include <algorithm>
#include <math.h>
#include <forward_list>
#include <unistd.h>

#include "curses.h"

struct Node {
    int value;
    struct Node *next;
};

struct Head {
    struct Node *next;
};


void up(int n) {
    std::cout << "\033[" << n << "A";
}

void down(int n) {
    std::cout << "\033[" << n << "B";
}

void right(int n) {
    std::cout << "\033[" << n << "C";
}

void left(int n) {
    std::cout << "\033[" << n << "D";
}

enum VT_100_color {
    black = 40,
    dark_red,
    green,
    yellow,
    blue,
    purple,
    dark_green,
    white
};

void set_color(enum VT_100_color color) {
    std::cout << "\033[0m\033[" << color << "m";
}

void exit_function(int sig) {
    std::cout << "\033[2J\033[?25h\033[0;0H\033[0m";
    echo();
    endwin();
    exit(0);
}

void display_block(int x, int y) {
    if (x >= 0 && y >= 0) {
        std::cout << "\033[15A";
        down(x);
        right(y);
        set_color(black);
        std::cout << "  ";
        up(x);
        left(y + 2);
    }
}

void deal_key() {
    int key;
    key = getchar();
    std::cout << "getchar();";
    static int x = 0;
    static int y = 0;

    if (key != ERR && key != 'q') {
        clrtoeol();
        switch (key) {
            case KEY_UNDO:
                ++x;
                break;
            case KEY_UP:
                --x;
                break;
            case KEY_RIGHT:
                ++y;
                break;
            case KEY_LEFT:
                --y;
                break;
        }
    }
    display_block(x, y);
}

int main(int argc, char **argv) {
    std::forward_list<int> fl;
    fl.push_front(10);
    fl.push_front(20);
    fl.push_front(30);
    signal(SIGINT, exit_function);
//    initscr();
//    noecho();


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
    while (1) {
        for (int i = 0; i < 15; ++i) {
            for (int i = 0; i < 30; ++i)
                std::cout << "\033[41m" << ' ' << "\033[0m";
            std::cout << "\r\n\033[0m";
        }

//        display_block(2, 4);
//        deal_key();
        std::cout << "\033[2;0H";
        std::cout.flush();
        char tem[20];

        if (tem[0] == 'q'){
            std::cout << "12343";
        }

        //        char tem;
//        std::cin >> tem;
//        if (tem == 'w')
//            std::cout << "\033[0m" << " out w" << std::endl;
    }
    endwin();
}
