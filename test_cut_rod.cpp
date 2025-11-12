//
// Created by 潘鑫 on 2025/2/18.
//
#include <gtest/gtest.h>


int price[] = {0, 1, 5, 8, 9, 10, 17, 17, 20, 24, 30};

int cut_rod(const int price[], int n);

int memorized_cut_rod(const int price[], int n);

//int pri = memorized_cut_rod(price, 7);
//std::cout << "价格 : " << pri << std::endl;

TEST(cut_rol_test, max) {
    //    EXPECT_EQ(MIN(9, 4), 4);
    //    EXPECT_EQ(cut_rod(price, 25), 9);
    // EXPECT_EQ(memorized_cut_rod(price, 25), 9);
}


TEST(vecter, add) {
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<int> expected = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    // for (int v1: v) {
    //     expected.push_back(v1);
    // }
    EXPECT_EQ(v, expected);  // 居然是可以直接判断两个vector 是否相等的
}
