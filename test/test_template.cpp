//
// Created by 潘鑫 on 2025/9/25.
//
#include  <gtest/gtest.h>

template<typename T>
static inline T ImMin(T lhs, T rhs) { return lhs < rhs ? lhs : rhs; }


struct a {
    int lhs;
    int rhs;
};

static inline a ImMin(a lhs, a rhs) { return lhs.lhs < lhs.rhs ? lhs : rhs; }


TEST(min, template_min) {
    a te{1, 2};
    a teb{3, 4};
    a dd= ImMin(te,teb);
    EXPECT_EQ(ImMin(1,2), 1);
    EXPECT_EQ(ImMin(1,2), 1);
}
