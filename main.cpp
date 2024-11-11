#include <iostream>

#include <thread>

void printMessage(int count) {
    for (int i = 0; i < count; ++i) {
        std::cout << "hello from thread (function pointer)" << std::endl;
    }
}

int main() {
    std::thread t1(printMessage, 5);
    t1.join();
    return 0;

}