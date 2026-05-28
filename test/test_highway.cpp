//
// Created by 潘鑫 on 2026/5/28.
//

#include <iostream>
#include <vector>

#include <gtest/gtest.h>


#include <hwy/highway.h>

#include "hwy/ops/set_macros-inl.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <numeric>
#include <vector>


#include "hwy/highway.h"

HWY_BEFORE_NAMESPACE();

namespace hwy {
    namespace
    HWY_NAMESPACE {
        namespace hn = hwy::HWY_NAMESPACE;

        float SumArraySIMD(const float * HWY_RESTRICT array, size_t count) {
            const hn::ScalableTag<float> d;
            using V        = hn::Vec<decltype(d)>;
            V sum          = hn::Zero(d);
            size_t i       = 0;
            const size_t N = hn::Lanes(d);
            if (count >= N) {
                for (; i <= count - N; i += N) {
                    sum = hn::Add(sum, hn::LoadU(d, array + i));
                }
            }
            float total = hn::ReduceSum(d, sum);
            // Simple scalar remainder handling
            for (; i < count; ++i) {
                total += array[i];
            }
            return total;
        }
    } // namespace HWY_NAMESPACE
}     // namespace hwy
HWY_AFTER_NAMESPACE();
namespace hn = hwy::HWY_NAMESPACE;

#include <hwy/contrib/algo/transform-inl.h>

TEST(highway, add) {
    const size_t size = 1000;
    // std::vector<float> a(size, 1.0f);
    // std::vector<float> b(size, -2.0f);
    // std::vector<float> c(size, 0.0f);
    std::vector<float> result(size, 0.0f);

    // auto total = hwy::HWY_NAMESPACE::SumArraySIMD(a.data(), size);
    std::vector<float> a       = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    const std::vector<float> b = {10.0f, 20.0f, 30.0f, 40.0f, 50.0f};
    const std::vector<float> c(5, 4.0f); // 存放结果

    const hn::ScalableTag<float> d;

    // 2. 直接调用 Transform 二元运算函数，完全不需要自己写循环！
    // 参数含义: (数据标签, 数组A开头, 数组A结尾, 数组B开头, 结果C开头, SIMD操作Lambda)
    hn::Transform2(d, a.data(), a.size(), // 输入1 的指针和总长度
                   b.data(),              // 输入2 的指针
                   c.data(),              // 输出 的指针
                   [](auto d, auto a, auto v_a, auto v_b) {
                       return hn::Add(v_a, v_b); // 核心 SIMD 运算
                   }); // 确实是一个很细节的函数
    EXPECT_EQ(c.size(), result.size());
    EXPECT_EQ(c, result);
}
