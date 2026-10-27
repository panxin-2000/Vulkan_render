//
// Created by 潘鑫 on 2026/10/27.
//
#include <stdio.h>
#include <unistd.h> // 提供 sleep/usleep 函数


#include "tracy/Tracy.hpp" // NOLINT(bugprone-reserved-identifier)


// 模拟一个高耗时的计算函数
void HeavyCalculation() {
    // 1. 自动以当前函数名 "HeavyCalculation" 创建一个性能监控区域
    // 利用 RAII 机制，当函数结束（走出这个花括号）时，该区域自动闭合
    ZoneScoped;
    // ZoneScopedNC("Network Sync", 0xFF0000); // 在 Tracy 里显示为红色的方块 , 不能同时出现两个,会有冲突


    printf("Starting heavy calculation...\n");
    usleep(2000000); // 模拟耗时 200 毫秒
}

int main(int argc, char **argv) {
    ZoneScopedN("OpenCL Init");


    // 循环 50 次，方便你在 Tracy GUI 中观察连续的帧或波形
    while (1) {
        // 3. 标记每一轮循环（在 Tracy 中会显示为 Frame/帧）
        FrameMark;

        HeavyCalculation();
        usleep(16000); // 模拟每帧之间的空闲时间 (约 16ms)
    }

    return 0;
}
