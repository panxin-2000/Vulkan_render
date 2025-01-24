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

static inline int ImUpperPowerOfTwo(int v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

static inline int ImLowerPowerOfTwo(int v) {
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v>>1;  // 异或运算符是不行的，因为要把最高位后面全部变成0 最高位左移一位然后得到一个新的数，然后不断或，最好再异或应该是可以的
    //异或是不行的
}

int main(int argc, char **argv) {
    std::cout << "ImUpperPowerOfTwo( 0 ) = " << ImUpperPowerOfTwo(0) << std::endl;
    std::cout << "ImUpperPowerOfTwo( 1 ) = " << ImUpperPowerOfTwo(1) << std::endl;
    std::cout << "ImUpperPowerOfTwo( 2 ) = " << ImUpperPowerOfTwo(2) << std::endl;
    std::cout << "ImUpperPowerOfTwo( 3 ) = " << ImUpperPowerOfTwo(3) << std::endl;
    std::cout << "ImUpperPowerOfTwo( 4 ) = " << ImUpperPowerOfTwo(4) << std::endl;
    std::cout << "ImUpperPowerOfTwo( 5 ) = " << ImUpperPowerOfTwo(5) << std::endl;
    std::cout << "ImUpperPowerOfTwo( 129 ) = " << ImUpperPowerOfTwo(129) << std::endl;
    // 一个很特殊的例子，129能够充分的演示为什么是这个结果。
    std::cout << "ImLowerPowerOfTwo( 0 ) = " << ImLowerPowerOfTwo(0) << std::endl;
    std::cout << "ImLowerPowerOfTwo( 1 ) = " << ImLowerPowerOfTwo(1) << std::endl;
    std::cout << "ImLowerPowerOfTwo( 2 ) = " << ImLowerPowerOfTwo(2) << std::endl;
    std::cout << "ImLowerPowerOfTwo( 3 ) = " << ImLowerPowerOfTwo(3) << std::endl;
    std::cout << "ImLowerPowerOfTwo( 4 ) = " << ImLowerPowerOfTwo(4) << std::endl;
    std::cout << "ImLowerPowerOfTwo( 5 ) = " << ImLowerPowerOfTwo(5) << std::endl;
    std::cout << "ImLowerPowerOfTwo( 8 ) = " << ImLowerPowerOfTwo(8) << std::endl;
    std::cout << "ImLowerPowerOfTwo( 129 ) = " << ImLowerPowerOfTwo(129) << std::endl;
    // 一个很特殊的例子，129能够充分的演示为什么是这个结果。


}
