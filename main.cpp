#include <iostream>

#include <thread>

void increment(int &x) {
    ++x;
}

int main() {
    int num = 0;
    std::thread t1(increment, std::ref(num));
    t1.join();
    std::cout << "value after increment " << num << std::endl;
    return 0;

}