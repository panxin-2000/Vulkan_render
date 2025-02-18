//
// Created by 潘鑫 on 2025/2/17.
//

#include <cassert>
#include <iostream>
#include <queue>


struct S {
    int id;

    S(int i, double d, std::string s) : id{i} {
        std::cout << "S::S" << i << ", " << d << ", \"" << s
                  << "\"); " << std::endl;

    }
};

void test_queue_emplace() {
    std::queue<S> queue;
    const S &s = queue.emplace(42, 3.14, "C++");
    std::cout << "id = " << s.id << std::endl;
}

void test_run_queue() {
    test_queue_emplace();

    std::queue<int> q;
    q.push(0);
    q.push(1);
    q.push(2);
    q.push(3);
    assert(q.front() == 0);
    assert(q.back() == 3);
    assert(q.size() == 4);

    q.pop();
    assert(q.size() == 3);

    std::cout << "q : ";
    for (; !q.empty(); q.pop())
        std::cout << q.front() << ' ';
    std::cout << std::endl;
    assert(q.size() == 0);
}

