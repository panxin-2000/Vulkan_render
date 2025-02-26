//
// Created by 潘鑫 on 2025/2/5.
//
//
#include <gtest/gtest.h>


#define MIN(x, y)({ typeof(x)_x = (x);typeof(y)_y = (y);(void)(&_x==&_y);_x<_y?_x:_y;})
#define MAX(x, y)({ typeof(x)_x = (x);typeof(y)_y = (y);(void)(&_x==&_y);_x>_y?_x:_y;})

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
    // Expect two strings not to be equal.
    EXPECT_STRNE("hello", "world");
    //    EXPECT_STRNE("hello", "hello");
    //    Expected: ("hello") != ("hello"), actual: "hello" vs "hello"

    // Expect equality.
    EXPECT_EQ(7 * 6, 42);
}


TEST(min_of_two_number, min) { {
        EXPECT_EQ(MIN(9, 4), 4);
    } {
        EXPECT_EQ(MAX(9, 4), 9);
    }
}

TEST(min_of_two_number, max) {
    //    EXPECT_EQ(MIN(9, 4), 4);
    EXPECT_EQ(MAX(9, 4), 9);
}

struct ss {
    int a;
    int b;
    float c;
    double d;
    bool  operator==(const ss &r) const{ return a == r.a && b == r.b && c == r.c && d == r.d; };
};

TEST(ss, struct_eqeue) {
    struct ss s = {1, 2, 3.0, 4.6};
    struct ss sc = {1, 2, 3.0, 4.6};
    EXPECT_EQ(s, sc);  // 其实还是调用了C++中的代码，是不是不知方便显示结果或者组合测试示例？
}
