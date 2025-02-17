//
// Created by 潘鑫 on 2025/2/17.
//

#include <functional>
#include <iostream>
#include <queue>
#include <string_view>
#include <vector>

template<typename T>
void pop_println(std::string_view rem, T &pq) {
    std::cout << rem << " :";
    for (; !pq.empty(); pq.pop()) {

        std::cout << pq.top() << ' ';
        // 上一行的pq.pop() 不返回任何内容void,上面完全可以分为两行。
        // 上一行不应该是pq.pop() ,而应该是 pq.top()
    }
    std::cout << std::endl;
}

/**
 * 这个函数也很有意思，T不需要指定，如何实现的？
 * @tparam T
 * @param rem
 * @param v
 */
template<typename T>
void print_ln(std::string_view rem, const T &v) {
    std::cout << rem << " :";
    for (const auto &e: v)
        std::cout << e << ' ';
    std::cout << std::endl;
}

void test_priority_queue() {
    const auto data = {1, 8, 5, 6, 3, 4, 0, 9, 7, 2};
    print_ln("data", data);

    std::priority_queue<int> max_priority_queue;

    //fill the priority queue
    for (const int &n: data) { //这一行原本是  for ( int n: data) // 改和不改都不影响运行速度
        max_priority_queue.push(n);
    }
    pop_println("max_priority_queue", max_priority_queue);

    //std::greater<int> makes the max priority queue act as a min priority queue
    std::priority_queue<int, std::vector<int>, std::greater<int>>
            min_priority_queue1(data.begin(), data.end());
    pop_println("min_priority_queue", min_priority_queue1);

    //second way to define a min priority queue
    std::priority_queue min_priority_queue2(data.begin(), data.end(), std::greater<int>());
    // 抄代码的时候 std::greater<int> 后面少了一个括号，然后出现了一个报错

    pop_println("min_priority_queue 2 ", min_priority_queue2);

    // using a custom function object to compare elements.
    struct {
        bool operator()(const int l, const int r) const { return l > r; }
    } customLess;
    std::priority_queue custom_priority_queue(data.begin(), data.end(), customLess);

    pop_println("custom_priority_queue", custom_priority_queue);

    //use lambda to compare elements
    auto cmp = [](int left, int right) { return (left ^ 1) < (right ^ 1); };
    //                                                                    逻辑异或操作

    std::priority_queue<int, std::vector<int>, decltype(cmp)> lambda_priority_queue(cmp);
    // 这里的 decltype 我之前好像都没有见过这个关键字
    for (int n: data) {
        lambda_priority_queue.push(n);
    }
    pop_println("lambda_priority_queue", lambda_priority_queue);
}
