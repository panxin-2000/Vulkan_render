#include <iostream>
#include <vector>
#include <thread>
#include <string>

int count = 0;

void doSomeWork() {
    std::cout << "the doSomeWork function is running on another thread" << std::endl;
    int data = count++;

    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::string str = std::to_string(data);
    std::cout << "the function call by the worker thread has ended. " + str << std::endl;
}


int main() {
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.push_back(std::thread(doSomeWork));
        std::cout << "the main thread calls this after starting the new tread" << std::endl;
    }
    for (auto &thread: threads) {
        thread.join();
    }
    return 0;

}