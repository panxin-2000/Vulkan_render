#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <filesystem>
#include <random>
#include "run_time.h"

struct Test {
    uint32_t length;
    std::vector<std::string> tem;  //vector 占据了6个字节
    std::vector<uint32_t> le;
} test;

class run {
private:
    int32_t tm;
    int32_t tmd;
public:
    uint32_t ren;
    uint32_t rden;

    run() {
        std::cout << "init " << std::endl;
    }
};


int main() {
    std::cout << "size of test " << sizeof(test) << std::endl;
//    test.tem.push_back("sdsdsds");
    std::cout << "size of test " << sizeof(test) << std::endl;
//    test.le.push_back(35);
    std::cout << "size of test " << sizeof(test) << std::endl;
    std::vector<uint32_t> lem;
    std::cout << "size of lem " << sizeof(lem) << std::endl;
    lem.push_back(24);
    lem.push_back(24);
    lem.push_back(24);
    std::cout << "size of lem " << sizeof(lem) << lem[1] << std::endl;
    // lem[1] 这里开起来是c的操作，但是实际上是另一个种操作，因为向量的长度是固定的，不管添加的参数是什么。
    class run rd;
    std::cout << "size of rd " << sizeof(rd) << std::endl;


    return 0;
}