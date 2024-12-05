//
// Created by 潘鑫 on 2024/12/5.
//
#include "run_time.h"


uint64_t Run_time::get_delta_time_ns() const{
    auto end_time = std::chrono::steady_clock::now();
    uint64_t delta = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - this->start_time).count();
    return delta;
}