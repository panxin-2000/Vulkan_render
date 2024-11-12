#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>

std::mutex mtx;
int shared_resource = 0;

void print_message(const std::string &message, int delay) {
    std::unique_lock<std::mutex> lock(mtx);
    ++shared_resource;
    lock.unlock(); //这里其实是稍微法线来一点问题的，那就是如果你不存储临时计算的结果，之后结果是会变的。
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    std::cout << message << shared_resource << std::endl;
}

int main() {
    std::vector<std::thread> tem_threads;
    tem_threads.push_back(std::thread(print_message, "hello 1 ", 500));
    tem_threads.push_back(std::thread(print_message, "hello 2 ", 1000));
    for (auto &tem_thread: tem_threads) {
        if (tem_thread.joinable())
            tem_thread.join();
    }
    std::cout << "main thread finish. " << std::endl;
    return 0;

}