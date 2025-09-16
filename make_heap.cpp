//
// Created by 潘鑫 on 2025/2/19.
//
#include <algorithm>
#include <functional>
#include <iostream>
#include <string_view>
#include <vector>

void print(std::string_view text, const std::vector<int> &v = {}) {
    std::cout << text << ": ";
    for (const auto &i: v) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}

void test_make_heap() {
    print("Max heap: ");
    std::vector<int> v{3, 2, 4, 1, 5, 9};
    print("initially , v", v);
    std::make_heap(v.begin(), v.end());
    print("after make heap, v", v);

    std::pop_heap(v.begin(), v.end());
    print("after pop heap ,v ", v);

    auto top = v.back();
    v.pop_back();
    print("former top element", {top});
    print("afer removing the former top element, v", v);

    print("\n MIn heap");

    std::vector<int> v2 = {3, 2, 4, 1, 5, 9};
    print("initially , v2", v2);

    std::make_heap(v2.begin(), v2.end(), std::greater<int>());
    print("after make heap, v2", v2);

    std::pop_heap(v2.begin(), v2.end(), std::greater<int>());
    print("after pop heap, v2", v2);

    auto top2 = v2.back();
    v2.pop_back();
    print("former top element", {top2});
    print("afer removing the former top element, v2", v2);


    struct df {
        int x;
        int y;
        int distance;
        bool operator()(const df *l, const df *r) const { return l->distance > r->distance; }
    };

    std::vector<int> v3 = {3, 2, 4, 1, 5, 9};
    std::vector<df *> e;
    for (int i = 0; i < v3.size(); ++i) {
        auto a = new df;
        a->distance = v3[i];
        e.push_back(a);
    }

    std::make_heap(e.begin(), e.end(), [](df *left, df *right) { return left->distance < right->distance; });


    for (auto v1: e) {
        std::cout <<  e.back()->distance <<" ";
    }
    std::cout << std::endl;

    while (!e.empty()) {
        // std::make_heap(e.begin(), e.end(), [](df *left, df *right) { return left->distance < right->distance; });
        std::pop_heap(e.begin(), e.end());
        std::cout << "last : " << e.back()->distance << std::endl;
        e.pop_back();
    }

    // pop_heap 并不会去重新排序为一个堆，为什么没有呢？

    std::cout << std::endl;
}

// 两件事，make_heap 只是一个算法，并不是一个容器或其他
// 另外 clion的新的新版确实很好，很聪明的识别出来了上面的模式，然后能够创建出下面min 部分的内容

// 然后就是comp的内容，其实之前也碰到过，但是呢？没有去写结构体的比较，现在看看应该怎么写
