#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>

std::mutex mtx;
std::condition_variable cv;
bool ready = false;

void print_id(const int id) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [] { return ready; });
    std::cout << "thread id : " << id << std::endl;
}

void set_ready() {
    std::unique_lock<std::mutex> lock(mtx);//我现在不理解这里到底做了什么？
    ready = true;
    cv.notify_all();
}

int main() {
    std::vector<std::thread> tem_threads;
    tem_threads.push_back(std::thread(print_id, 1));
    tem_threads.push_back(std::thread(print_id, 2));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    set_ready();
    for (auto &tem_thread: tem_threads) {
        if (tem_thread.joinable())
            tem_thread.join();
    }
    std::cout << "main thread finish. " << std::endl;
    return 0;

}