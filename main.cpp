#include <iostream>
#include <mutex>
#include <string>
#include <thread>

volatile int g_i = 0;
std::mutex g_i_mutex;

void safe_increment(int iterations) {
    const std::lock_guard<std::mutex> lock(g_i_mutex);
    while (iterations-- > 0) {//这里稍微有个问题，那么就是执行的优先级--先还是>号先执行
        g_i += 1;
    }
    std::cout << "thread #" << std::this_thread::get_id() << ",g_i: " << g_i << std::endl;

}

void unsafe_increment(int iterations) {
    while (iterations-- > 0) {//这里稍微有个问题，那么就是执行的优先级--先还是>号先执行
        g_i += 1;
    }
    std::cout << "thread #" << std::this_thread::get_id() << ",g_i: " << g_i << std::endl;
}

int main() {
    auto test = [](std::string_view fun_name, auto fun) {
        g_i = 0;
        std::cout << fun_name << ":\nbefore ,g_i: " << g_i << std::endl;

        std::thread t1(fun, 1'000'000);
        std::thread t2(fun, 1'000'000);
        t1.join();
        t2.join();
        std::cout << "after,g_i: " << g_i << std::endl;


    };
    test("safe_increment", safe_increment);
    test("unsafe_increment", unsafe_increment);

}