//
// Created by 潘鑫 on 2026/5/28.
//

#include <gtest/gtest.h>

#include "base_geometry/geometry_element/point_3.h"

#include <Eigen/Eigen>
#include <utility>
#include "hwy/highway.h"
#include "hwy/contrib/algo/transform-inl.h"


inline float get_fast_random_float() {
    // 线程独立的 32 位种子
    static thread_local uint32_t state = 2463534242u;

    // Xorshift 核心高速位移指令
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;

    // IEEE 754 浮点数黑魔法：将 32 位整数直接映射到 [0.0f, 1.0f) 的 float
    // 这种方法避免了传统 (float)rand() / RAND_MAX 的慢速硬件整数除法
    return (state & 0xFFFFFF) / 16777216.0f;
}

struct ScopeTimer {
    std::string name;
    std::chrono::high_resolution_clock::time_point start;

    explicit ScopeTimer(std::string scope_name)
        : name(std::move(scope_name)), start(std::chrono::high_resolution_clock::now()) {
    }

    ~ScopeTimer() {
        const auto end = std::chrono::high_resolution_clock::now();
        std::cout << "[Timer] " << name << " 耗时: "
                << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns\n";
    }
};


namespace hn = hwy::HWY_NAMESPACE;

TEST(particle, matrix) {
    const size_t size = 10000; {
        std::vector<Point_3> position(size);
        for (size_t i = 0; i < size; ++i) {
            position[i] = {1.0f, 1.0f, 1.0f};
            position[i] = position[i] * Point_3{
                              get_fast_random_float(), get_fast_random_float(), get_fast_random_float()
                          };
        }
        std::vector<Point_3> speed(size);
        for (size_t i = 0; i < size; ++i) {
            speed[i] = Point_3{get_fast_random_float(), get_fast_random_float(), get_fast_random_float()};
        } {
            ScopeTimer particle("simple particle ");
            for (size_t time = 0; time < size; ++time)
                for (size_t i = 0; i < size; ++i) {
                    position[i] = position[i] + speed[i];
                }
        }
    } {
        std::vector<Eigen::Vector4f> position(size);

        for (size_t i = 0; i < size; ++i) {
            position[i] = {1.0f, 1.0f, 1.0f, 1.0f};
            position[i] = position[i].array() * Eigen::Vector4f(get_fast_random_float(),
                                                                get_fast_random_float(),
                                                                get_fast_random_float(),
                                                                1.0f).array();
        }
        std::vector<Eigen::Vector4f> speed(size);
        for (size_t i = 0; i < size; ++i) {
            speed[i] = Eigen::Vector4f{
                get_fast_random_float(),
                get_fast_random_float(),
                get_fast_random_float(), 1.0f
            };
        } {
            ScopeTimer particle("eigen particle  ");
            for (size_t time = 0; time < size; ++time)
                for (size_t i = 0; i < size; ++i) {
                    position[i] = position[i] + speed[i];
                }
        } {
            ScopeTimer particle("highway particle");
            for (size_t time = 0; time < size; ++time) {
                const hn::ScalableTag<float> d;
                // 参数含义: (数据标签, 数组A开头, 数组A结尾, 数组B开头, 结果C开头, SIMD操作Lambda)
                hn::Transform2(d, position.data()->data(), 4 * position.size(), // 输入1 的指针和总长度
                               position.data()->data(),                         // 输入2 的指针
                               speed.data()->data(),                            // 输出 的指针
                               [](auto d, auto a, auto v_a, auto v_b) {
                                   return hn::Add(v_a, v_b); // 核心 SIMD 运算
                               });                           // 确实是一个很细节的函数
            }
            for (size_t i = 0; i < size; ++i) {
                position[i] = position[i] + speed[i];
            }
        }
    }


    int a = 10;
}
