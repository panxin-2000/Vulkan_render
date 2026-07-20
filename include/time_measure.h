//
// Created by 潘鑫 on 2026/7/11.
//

#ifndef HELLO_MAC_TIME_MEASURE_H
#define HELLO_MAC_TIME_MEASURE_H


#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>

class ScopedTimer {
public:
    explicit ScopedTimer(std::string name)
        : m_name(std::move(name)), m_start(std::chrono::high_resolution_clock::now()) {
    }

    ~ScopedTimer() {
        intermediate_record("end");
    }

    void intermediate_record(const std::string &intermediate_record) {
        // 统一先拿到最细粒度的纳秒数 (nanoseconds)
        const auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>
                (std::chrono::high_resolution_clock::now() - m_start).count();
        std::cout << "[Timer] " << m_name << " : " << intermediate_record << " took ";
        print_duration(duration_ns);
    }

    ScopedTimer(const ScopedTimer &) = delete;

    ScopedTimer &operator=(const ScopedTimer &) = delete;

private:
    void print_duration(const long long duration_ns) {
        // 设置输出浮点数时保留两位小数
        std::cout << std::fixed << std::setprecision(2);
        // 自适应单位分流逻辑
        if (duration_ns >= 1'000'000'000) {
            // 大于等于 1 秒
            std::cout << static_cast<double>(duration_ns) / 1'000'000'000.0 << " s\n";
        } else if (duration_ns >= 1'000'000) {
            // 大于等于 1 毫秒
            std::cout << static_cast<double>(duration_ns) / 1'000'000.0 << " ms\n";
        } else if (duration_ns >= 1'000) {
            // 大于等于 1 微秒
            std::cout << static_cast<double>(duration_ns) / 1'000.0 << " us\n"; // 终端常用 us 代替 μs
        } else {
            // 极快的情况，直接输出纳秒
            std::cout << static_cast<double>(duration_ns) << " ns\n";
        }
    }

    std::string m_name;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
};


#endif //HELLO_MAC_TIME_MEASURE_H
