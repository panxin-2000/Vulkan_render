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
    exit(0);
}

int main(int argc, char **argv) {
    signal(SIGINT, exit_function);
    int key;
    initscr();
    crmode();
    keypad(stdscr, TRUE);
    noecho();
    clear();
    refresh();
    key = getch();
    while (key != ERR && key != 'q') {
        move(7, 5);
        clrtoeol();
        if ((key >= 'A' && key <= 'Z')
            || (key >= 'a' && key <= 'z')) {
            std::cout << "Key was " << (char) key << std::endl;
        } else {
            switch (key) {
                case KEY_LEFT:
                    std::cout << "left key" << std::endl;
            }
        }
        refresh();
        key = getch();

    }
    endwin();
    exit(EXIT_SUCCESS);
}
