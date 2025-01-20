#include <iostream>
#include <string>
#include <map>
#include <algorithm>
#include <math.h>

typedef int KeyType;
typedef std::pair<const KeyType, std::string> Pair;
typedef std::multimap<KeyType, std::string> MapCode;
#define  length 11


int quick_sort(int *A, int start, int end);

int partition(int *A, int start, int end);

void printf_pi(int *src, int start, int end) {
    for (int i = start; i <= end; ++i) {
        std::cout << *(src + i);
    }
    std::cout << std::endl;
}

int main() {
    using namespace std;
    int pi[length];
    pi[0] = 0;
    double PI = acos(-1);
    for (int i = 1; i < length; ++i) {
        pi[i] = (int) PI % 10;
        PI = PI * 10;
    }
    printf_pi(pi, 1, length - 1);

    quick_sort(pi, 1, length - 1);
    printf_pi(pi, 1, length - 1);

    return 0;
}

void exchange(int *src, int *des) {
    int tem = *des;
    *des = *src;
    *src = tem;
}

// 我写for 还是习惯从零开始，实际上算法导论都是从1开始发
int partition(int *A, int start, int end) {
    int x = *(A + end);
    int little = start - 1;
    for (int big = start; big <= end; ++big) {
        if (*(A + big) <= x) {
            ++little;
            exchange(A + big, A + little);
        }
    }
    exchange(A + little + 1, A + end);
    return little;
}

int quick_sort(int *A, int start, int end) {
    if (start + 1 < end) {
        uint part = partition(A, start, end);
        quick_sort(A, start, part - 1);
        quick_sort(A, part + 1, end);
    }
}
