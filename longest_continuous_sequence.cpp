//
// Created by 潘鑫 on 2025/2/17.
//
//#include <iostream>
#include <vector>
#include <unordered_map>
#include "gtest/gtest.h"

int longest_consecutive(std::vector<int> &nums) {
    std::unordered_map<int, int> L, R; // 首先这里可以理解为两个hash表
    int ans = 0;

    // 首先哪个是键，哪个是值
    // []中的是键，L[]的左值是值
    for (auto x: nums) {
        if (L[x] || R[x])continue;

        int l = L[x - 1]; //
        int r = L[x + 1];
        L[x] = l + 1; // 有左边连续值，长度会变成2，没有左值，长度还是为一
        R[x] = r + 1;

        int d = l + r + 1; // 总值等于左边连续值加上右边连续值
        ans = std::max(ans, d);

        L[x + r] = R[x - l] = d;
        // x如果有右边连续值的话，才会在新的上更新其值
    }
    return ans;
}

/**
 * 最长连续序列，其中最重要的步骤取两边的连续序列的长度再加上自身的长度，得到最大的连续长度
 *                          然后再将最长长度更新到起始点和终止点，因为中间点第一次肯定会算的
 *                          但是中间点再次重现的时候不会再起作用了，所以被直接跳过了
 *                          原文好像也介绍我优化后的方法了
 * @param nums
 * @return
 */
int update_longest_consecutive(std::vector<int> &nums) {
    std::unordered_map<int, int> Length; // 首先这里可以理解为两个hash表
    int ans = 0;

    // 首先哪个是键，哪个是值
    // []中的是键，L[]的左值是值
    for (auto x: nums) {
        auto it = Length.find(x);
        auto ff = Length[x];
        if (Length[x])continue;

        int left = Length[x - 1]; //left 代表左边有几个连续值
        int right = Length[x + 1]; //right 代表右边有几个连续值
        int d = left + right + 1; // 总值等于左边连续值加上右边连续值
        ans = std::max(ans, d);

        Length[x + right] = Length[x - left] = d;
        // 只在两边更新其值
    }
    return ans;
}


TEST(hash, find) {
    std::vector<int> nums = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    update_longest_consecutive(nums);
}
