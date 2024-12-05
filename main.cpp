#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <filesystem>
#include <random>


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
        std::cout  << tem << " ";
        to_be_sorted.push_back(tem);
    }
    std::cout << std::endl;

    for (auto current = to_be_sorted.begin(); current != to_be_sorted.end(); ++current) {
        for (auto little = current+1; little != to_be_sorted.end(); ++little) {
            if (*current > *little) {
//                std::cout  << "前 ";
//                for (auto a: to_be_sorted) {
//                    std::cout << a << ' ';
//                }
//                std::cout << std::endl;
                auto tem = *current;
                *current = *little;
                *little = tem;
//                std::cout  << "后 ";
//                for (auto a: to_be_sorted) {
//                    std::cout << a << ' ';
//                }
//                std::cout << std::endl;
            }
        }
    }
    for (auto a: to_be_sorted) {
        std::cout << a << ' ';
    }
    std::cout << std::endl;

    // 5 7 2 3 9
    // 2 7 5 3 9
    // 2 5 7 3 9
    // 2 3 7 5 9
    // 2 3 5 7 9

    return 0;
}