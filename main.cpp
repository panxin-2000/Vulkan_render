#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <filesystem>
#include <random>
#include "run_time.h"

void print_vecter(std::vector<int> &tem) {
    std::cout << "前 ";
    for (auto a: tem) {
        std::cout << a << ' ';
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "current path is " << std::filesystem::current_path() / "asd" << std::endl;
    std::cout << "temp path is " << std::filesystem::temp_directory_path() << std::endl;
    std::cout << "temp path is " << std::filesystem::file_size(std::filesystem::current_path() / "main.cpp") << 'B'
              << std::endl;

    std::mt19937 generator;
    std::uniform_int_distribution<int> distribution(1, 100);

    std::vector<int> to_be_sorted;

    for (int i = 0; i < 35; ++i) {
        int tem = distribution(generator);
        std::cout << std::setw(3) << std::setfill(' ') << tem << " ";
        to_be_sorted.push_back(tem);
    }
    std::cout << std::endl;

    Run_time runtime;
//    for (auto current = to_be_sorted.begin(); current != to_be_sorted.end(); ++current) {
//        for (auto little = current + 1; little != to_be_sorted.end(); ++little) {
//            if (*current > *little) {
//                auto tem = *current;
//                *current = *little;
//                *little = tem;
//            }
//        }
////        print_vecter(to_be_sorted);
//    }
    std::sort(to_be_sorted.begin(), to_be_sorted.end());
    std::cout << runtime.get_delta_time_ns() << "ns" << std::endl;
    for (auto a: to_be_sorted) {
        std::cout << std::setw(3) << std::setfill(' ') << a << ' ';
    }
    std::cout << std::endl;

    //添加注释，时间确实是具有随机性的，确实需要找一个办法多测试几遍才能够得出一个比较平均的结果，滤除其中的某些概率性因素。

    // 5 7 2 3 9
    // 2 7 5 3 9
    // 2 5 7 3 9
    // 2 3 7 5 9
    // 2 3 5 7 9

    return 0;
}