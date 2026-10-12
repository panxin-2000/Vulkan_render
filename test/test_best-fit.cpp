//
// Created by 潘鑫 on 2026/10/11.
//

#include <gtest/gtest.h>
#include <vector>
#include <iostream>
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
    // 一个简单的测试逻辑,大致是对的 , 但是最后没有完全整合为一个大的空闲的块
}
