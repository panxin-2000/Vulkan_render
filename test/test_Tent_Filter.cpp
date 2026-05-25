//
// Created by 潘鑫 on 2026/5/26.
//

#include "gtest/gtest.h"


/**
 * 帐篷滤波器 中间（0附近）概率极高，两边（-1和1附近）概率线性递降
 * 但是给出的是概率？下面的公式是确定的公式
 * @param input 数值的输入范围在0～1之间
 * @return 返回值在 [ -1 ,1 ] 之间
 */
float Tent_Filter(const float input) {
    assert(input >= 0 && input <= 1 && "Tent_Filter need  input in 0~1");
    const auto temp = input * 2.0f;
    return temp < 1 ? sqrt(temp) - 1 : 1 - sqrt(2 - temp);
}

TEST(Filter, Tent) {

}
