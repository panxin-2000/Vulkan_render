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
    endwin();
    echo();
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
        clrtoeol();
        static int x = 10;
        static int y = 10;
        if ((key >= 'A' && key <= 'Z')
            || (key >= 'a' && key <= 'z')) {
//            std::cout << "Key was " << (char) key << std::endl;
        } else {
            switch (key) {
                case KEY_DOWN:
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
        move(x, y);
        printw("**");
        refresh();
        key = getch();
    }
    endwin();
    echo();
    exit(EXIT_SUCCESS);
}
