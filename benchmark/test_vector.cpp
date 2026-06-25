//
// Created by 潘鑫 on 2026/6/25.
//

#include <benchmark/benchmark.h>
#include <vector>

// 模拟测试：传统 vector 的遍历耗时
static void BM_VectorTraversal(benchmark::State &state) {
    const std::vector<int> v(state.range(0), 42); // 根据参数初始化 vector 大小

    // 核心循环：state 会自动控制迭代次数
    for (auto _: state) {
        long long sum = 0;
        for (const int x: v) {
            sum += x;
            benchmark::DoNotOptimize(sum); // 关键：防止编译器把无用循环优化掉
        }
    }
    state.SetItemsProcessed(state.range(0));
}

//  Range 的函数 按照 2的次方进行递增
//
BENCHMARK(BM_VectorTraversal)->Range(64, 8192) // 要求 1：必须有多个不同的数量（如 256, 1024, 4096, 8192）
    ->Complexity(benchmark::oN);               // 要求 2：必须显式开启复杂度分析，并给出你的预期（如 oN, oLogN）

BENCHMARK(BM_VectorTraversal)->Range(64, 8192) // 要求 1：必须有多个不同的数量（如 256, 1024, 4096, 8192）
    ->Complexity(benchmark::oN);               // 要求 2：必须显式开启复杂度分析，并给出你的预期（如 oN, oLogN）


// 运行所有的基准测试
BENCHMARK_MAIN();

// ./my_benchmark --benchmark_filter=BM_VectorTraversal 可以用来只跑我需要的这个测试 
