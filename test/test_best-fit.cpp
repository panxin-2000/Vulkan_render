//
// Created by 潘鑫 on 2026/10/11.
//

#include <gtest/gtest.h>
#include <vector>
#include <iostream>
#include <random>

#include "memory_alloc.h"


TEST(test_alloc, best_fit) {
    my_namespace::alloc_memory alloc(1024 * 64);
    auto result_1 = alloc.alloc(56);
    auto result_2 = alloc.alloc(57);
    auto result_3 = alloc.alloc(58);
    auto result_4 = alloc.alloc(59);
    auto result_5 = alloc.alloc(60);
    alloc.free(result_4.first);
    alloc.free(result_1.first);
    auto result_6 = alloc.alloc(59);
    alloc.free(result_2.first);
    alloc.free(result_3.first);
    alloc.free(result_5.first);
    alloc.free(result_6.first);
}

// 封装的填充函数
// 参数 1: 需要填充的 vector（传引用）
// 参数 2: 随机数范围的最小值 min
// 参数 3: 随机数范围的最大值 max
void fillVectorRandomRange(std::vector<uint32_t> &myVector, int min, int max) {
    // 静态局部变量，确保随机数引擎只初始化一次，性能极高且保证随机性
    static std::random_device rd;
    static std::mt19937 gen(rd());

    // 在函数内部利用参数 min 和 max 创建分布器
    std::uniform_int_distribution<int> distrib(min, max);

    // 传统的 for 循环填充
    for (size_t i = 0; i < myVector.size(); ++i) {
        myVector[i] = distrib(gen);
    }
}


std::vector<size_t> generate_random_indices(size_t size) {
    // 1. 初始化索引数组
    std::vector<size_t> indices(size);
    std::iota(indices.begin(), indices.end(), 0);

    // 2. 使用 static 保持随机引擎状态，避免重复创建带来的开销
    static std::random_device rd;
    static std::mt19937 g(rd());

    // 3. 洗牌打乱
    std::shuffle(indices.begin(), indices.end(), g);

    return indices;
}


TEST(test_alloc, best_fit_random) {
    std::vector<uint32_t> malloc_size;
    std::vector<std::pair<my_namespace::block_offset, my_namespace::alloc_size> > position;
    malloc_size.resize(1000);
    position.reserve(malloc_size.capacity());
    my_namespace::alloc_memory alloc(1024 * 64);

    fillVectorRandomRange(malloc_size, 1, 100);

    uint64_t total_size = std::accumulate(malloc_size.begin(), malloc_size.end(), 0ULL);

    for (size_t i = 0; i < malloc_size.size(); ++i) {
        position.push_back(alloc.alloc(malloc_size.at(i)));
    }

    auto random_index = generate_random_indices(position.size());

    for (size_t i = 0; i < random_index.size(); ++i) {
        alloc.free(position.at(i).first);
    }

    auto result = alloc.size();
    //
}
