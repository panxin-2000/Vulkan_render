//
// Created by 潘鑫 on 2024/12/5.
//
#include <chrono>

#ifndef HELLO_MAC_RUN_TIME_H
#define HELLO_MAC_RUN_TIME_H
using namespace std::chrono;

class Run_time {  //不记得哪里的规范好想说过类名称大小写的问题
public:
    //好像是曾经有人说过，C++你可以完全不使用其中复杂的内容，但是架不住库中已经在用了
    time_point<steady_clock, duration<long long, std::ratio<1LL, 1000000000LL>>> start_time;

    Run_time() {
        start_time = std::chrono::steady_clock::now();
    }
    uint64_t get_delta_time_ns() const;


};


#endif //HELLO_MAC_RUN_TIME_H
