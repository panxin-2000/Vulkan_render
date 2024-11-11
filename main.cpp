#include <iostream>
#include <thread>
#include <chrono>

void print_message(const std::string &message, int delay) {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    std::cout << message << std::endl;
}

int main() {
    std::vector<std::thread> tem_threads;
    tem_threads.push_back(std::thread(print_message, "hello 1", 1000));
    tem_threads.push_back(std::thread(print_message, "hello 2", 500));
    for (auto &tem_thread: tem_threads) {
        if (tem_thread.joinable())
            tem_thread.join();
    }
    std::cout << "main thread finish. " << std::endl;
    return 0;

}