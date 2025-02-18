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


TEST(min_of_two_number, min) {
{
EXPECT_EQ(MIN(9, 4), 4);
}
{
EXPECT_EQ(MAX(9, 4), 9);
}

}

TEST(min_of_two_number, max) {
//    EXPECT_EQ(MIN(9, 4), 4);
EXPECT_EQ(MAX(9, 4), 9);

}

