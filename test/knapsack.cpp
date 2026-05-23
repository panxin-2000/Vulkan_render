//
// Created by 潘鑫 on 2026/5/20.
//

#include <iostream>
#include <vector>
#include <algorithm>
#include <list>

#include <gtest/gtest.h>

using namespace std;

int knapsack01(const int back_wight, const vector<int> &weight, const vector<int> &value) {
    const auto number = weight.size();
    // dp[j] 表示容量为 j 的背包能装下的最大价值，初始化为 0
    vector<int> dp(back_wight + 1, 0);

    // 二维布尔矩阵，用来记录第 i 件物品在容量 j 时，【拿了】还是【没拿】
    // record[i][j] == true 代表拿了
    vector<vector<bool>> record(number, vector<bool>(back_wight + 1, false));

    // 遍历每一件物品
    for (int i = 0; i < number; i++) {
        // 逆序遍历背包容量，确保每件物品只拿一次,
        // 如果正序放的话，物品可能会被放多次，比如重量为5 的物品，在容量为10 时 又会被放一次
        for (int j = back_wight; j >= weight[i]; j--) {

            // 如果拿了新物品的组合价值更高
            if (dp[j - weight[i]] + value[i] > dp[j]) {
                dp[j] = dp[j - weight[i]] + value[i];
                record[i][j] = true; // 记录：在当前容量 j 下，第 i 件物品被装进去了
                // 之后就需要倒着计算了，
                // 从最后一个开始，依次向上，看看那个物品被放进去了
                // 从容量中减去该物品的重量，跳转到对于到 容量
                // 继续向上 直到 为零
            }
        }
        // 00 01 02 03 04
        // 00 00 00 00 04
        // 00 15 15 15 15
        // 00 15 15 20 35
        // 00 15 15 20 35
    }

    return dp[back_wight]; // 返回背包容量为 W 时的最大价值
}

TEST(knapsack, knapsack) {
    int back_wight    = 4;            // 背包最大容量
    vector<int> wight = {1, 3, 4};    // 物品重量
    vector<int> value = {15, 20, 30}; // 物品价值
    EXPECT_EQ(knapsack01(back_wight, wight, value), 35);
}
