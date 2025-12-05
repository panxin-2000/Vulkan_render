//
// Created by 潘鑫 on 2025/10/17.
//
#include <gtest/gtest.h>


#include "include/base_element/triangle.h"


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
bool if_colinear(Point_2 a, Point_2 b, Point_2 c) {
    Point_2 ab = b - a;
    Point_2 ac = c - a;
    float f1 = ab.single_area(ac);
    if (abs(f1) < 0.00001)
        return false;
    else
        return true;
}


TEST(centre, the_centre_of_a_circle) {
    Point_2 a{0, 3};
    Point_2 b{3, 6};
    Point_2 c{6, 3};
    Point_2 result{3, 3};

    EXPECT_EQ(true, if_colinear(a, b, c));
    EXPECT_EQ(result, Point_2::centre_of_a_circle(a, b, c));
}

TEST(centre, the_centre_of_a_circle_2) {
    Point_2 a{1, 3};
    Point_2 b{9, 1};
    Point_2 c{4, -2};
    Point_2 result{5, 2};


    struct MyStruct {
        using type = int; // 用 using 定义嵌套类型 type（等价于 typedef int type;）
    }; // 这个结构体的大小
    MyStruct::type f = 200;
    // 上面的f的本质应该还是int,主要的目的是什么其他的数据类型，可以被外部使用
    int g = f;
    // 目的也只是改一个名字，并不改变任何其他的内容

    EXPECT_EQ(true, if_colinear(a, b, c));
    EXPECT_EQ(result, Point_2::centre_of_a_circle(a, b, c));
}
