//
// Created by 潘鑫 on 2025/10/17.
//
#include <gtest/gtest.h>


#include "vector_signed_area.h"


// 输入三个点，之后呢？
// 求出三个点的圆心
// 怎么求呢？圆心到三个点的距离是相同的
// 第一步是需要判断是否是退化为共线的三个点
// 如果不共线，那么，再去求圆心
// 圆心到每个点的距离都相同

// $$ (x-x_a)^2+(y-y_a)^2=R^2 $$
// $$ x^2-2xx_a+x_a^2+y^2-2yy_a+y_a^2=R^2 $$
// $$ -2x(x_a-x_b)-2y(y_a-y_b)=x_b^2+y_b^2 - x_a^2 - y_a^2 $$
// $$ \begin{bmatrix}-2(x_a-x_b)&-2(y_a-y_b)\end{bmatrix}* \begin{bmatrix}x \\y\end{bmatrix}= x_b^2+y_b^2 - x_a^2 - y_a^2 $$
// $$ \begin{bmatrix}-2(x_a-x_c)&-2(y_a-y_c)\end{bmatrix}* \begin{bmatrix}x \\y\end{bmatrix}= x_c^2+y_c^2 - x_a^2 - y_a^2 $$
// $$ \begin{bmatrix}-2(x_a-x_b)&-2(y_a-y_b) \\-2(x_a-x_c)&-2(y_a-y_c) \end{bmatrix}* \begin{bmatrix}x \\y\end{bmatrix}= \begin{bmatrix} x_b^2+y_b^2 - x_a^2 - y_a^2\\x_c^2+y_c^2 - x_a^2 - y_a^2\end{bmatrix} $$
//  对下面这个矩阵求逆
// $$ $$ \begin{bmatrix}-2(x_a-x_b)&-2(y_a-y_b) \\-2(x_a-x_c)&-2(y_a-y_c) \end{bmatrix} $$
// 求逆之后与等式右边的矩阵相乘，得到最后的结果

/**
 *
 * @param a
 * @param b
 * @param c
 * @return
 */
bool if_colinear(segment_vector a, segment_vector b, segment_vector c) {
    segment_vector ab = b - a;
    segment_vector ac = c - a;
    float f1 = ab.single_area(ac);
    if (abs(f1) < 0.00001)
        return false;
    else
        return true;
}

segment_vector centre_of_a_circle(segment_vector a, segment_vector b, segment_vector c) {
    auto A_1_1 = -2 * (a.x - b.x);
    auto A_1_2 = -2 * (a.y - b.y);
    auto A_2_1 = -2 * (a.x - c.x);
    auto A_2_2 = -2 * (a.y - c.y);
    auto A = A_1_1 * A_2_2 - A_1_2 * A_2_1;
    auto inv_A_1_1 = A_2_2 / A;
    auto inv_A_1_2 =-1 * A_1_2 / A;
    auto inv_A_2_1 =-1 * A_2_1 / A;
    auto inv_A_2_2 = A_1_1 / A;
    auto B_1 = b.x * b.x + b.y * b.y - a.x * a.x - a.y * a.y;
    auto B_2 = c.x * c.x + c.y * c.y - a.x * a.x - a.y * a.y;

    segment_vector result = {};
    result.x = inv_A_1_1 * B_1 + inv_A_1_2 * B_2;
    result.y = inv_A_2_1 * B_1 + inv_A_2_2 * B_2;
    return result;
}


TEST(centre, the_centre_of_a_circle) {
    segment_vector a{0, 3};
    segment_vector b{3,6};
    segment_vector c{6, 3};
    segment_vector result{3, 3};

    EXPECT_EQ(true, if_colinear(a, b, c));
    EXPECT_EQ(result, centre_of_a_circle(a, b, c));
}
TEST(centre, the_centre_of_a_circle_2) {
    segment_vector a{1, 3};
    segment_vector b{9,1};
    segment_vector c{4, -2};
    segment_vector result{5, 2};

    EXPECT_EQ(true, if_colinear(a, b, c));
    EXPECT_EQ(result, centre_of_a_circle(a, b, c));
}
