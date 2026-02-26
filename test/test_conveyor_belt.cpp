//
// Created by 潘鑫 on 2026/2/26.
//

// 写这个文件是了测试传送带上物品向前移动的逻辑
// 举例如下：
// uint8_t 中的 1～255表示传送带格子上 有 物品
// uint8_t 中的 0     表示传送带格子上 无 物品
// 有物品时每次时间增加一，物品向前移动一
// 传送带上的两个物品不能穿模
// 那么这里的要求是，假设上一个物品的位置是15，那么下一个物品的最大位置值也是15

// 有一个位运算的程序会更好
#include "gtest/gtest.h"
#include <array>

TEST(conveyor_belt, single_direction) {
    constexpr size_t size = 100;
    std::array<uint8_t, size> single_direction{};
    single_direction[0] = 0xff;
    for (size_t i = 1; i < single_direction.size(); ++i) {
        if (single_direction[i] == 0xff && single_direction[i - 1] == 0x00) {
            // 当传送格已经到达最前段，并切上一格为空
            single_direction[i - 1] = 0x01; // 传送带向前移动一格
            single_direction[i]     = 0x00; // 当前移动一格之后清空当前格
        } else if (single_direction[i] != 0x00) {
            single_direction[i] += 1; // 传送带当前格子中的比例增加1
            if (single_direction[i] > single_direction[i - 1]) {
                single_direction[i] = single_direction[i - 1];
            }
        }
    }
}

// 逻辑不一定对

TEST(conveyor_belt, circle_direction) {
    constexpr size_t size = 100;
    size_t actual_size    = 10;
    std::array<uint8_t, size> single_direction{};
    if (actual_size == single_direction.size()) {
        for (unsigned char &i: single_direction) {
            i = i + (i != 0x00) + (i == 0xff);
        }
    } else {
        for (size_t i = 0; i < single_direction.size(); ++i) {
            size_t last_one = (i + single_direction.size() - 1) % single_direction.size();
            if (single_direction[i] == 0xff && single_direction[last_one] == 0x00) {
                // 当传送格已经到达最前段，并切上一格为空
                single_direction[last_one] = 0x01; // 传送带向前移动一格
                single_direction[i]        = 0x00; // 当前移动一格之后清空当前格
            } else if (single_direction[i] != 0x00) {
                single_direction[i] += 1; // 传送带当前格子中的比例增加1
            }
        }
    }
}
