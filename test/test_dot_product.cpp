//
// Created by 潘鑫 on 2025/3/9.
//
#include "Eigen/Eigen"
#include "gtest/gtest.h"
#include <vector>


TEST(dot_product, dot) {
    // 这里做了那些测试
    // 首先是一个简单的填充，之后是对行列式的值进行的计算
    // 然后对矩阵进行转置之后判断行列式的值是否相等
    Eigen::RowVector3f u(1, -2, 4);
    Eigen::RowVector3f v(-5, 2, 0);
    Eigen::RowVector3f w(1, 0, 3);
    Eigen::Matrix3f m{{1, -2, 4}, {-5, 2, 0}, {1, 0, 3}};
    Eigen::Matrix3f m2;
    m2 << u, v, w;

    ASSERT_EQ(m, m2);
    auto a = m.determinant();
    auto dot = v.dot(u);
    ASSERT_EQ(a, -32.0f);
    auto b = m.transpose();
    a = b.determinant();
    ASSERT_EQ(a, -32.0f);
}


bool back_or_forward(Eigen::RowVector3f a, Eigen::RowVector3f b) {
    Eigen::RowVector3f c(1, 1, 1);
    Eigen::Matrix3f m;
    m << a, b, c;
    auto dot = m.determinant();
    if (dot < 0.0f) {
        // 大于零是逆时针，小于零是顺时针
        return false;
    } else {
        return true;
    }
}

bool back_or_forward(Eigen::RowVector2f a, Eigen::RowVector2f b) {
    auto c = a[0] * b[1] - a[1] * b[0];
    if (c < 0.0f) {
        return false;
    } else {
        return true;
    }
}

TEST(tri, tri) {
    // 还是简单的测试完成了
    std::vector<Eigen::RowVector3f> tri;
    tri.push_back({1, 2, 3});
    tri.push_back({3, 2, 3});
    tri.push_back({2, 4, 3});
    auto a = static_cast<Eigen::RowVector3f>(tri[1].array() - tri[0].array());
    auto b = static_cast<Eigen::RowVector3f>(tri[2].array() - tri[1].array());
    auto c = static_cast<Eigen::RowVector3f>(tri[0].array() - tri[2].array());
    ASSERT_TRUE(back_or_forward(a,b));
    ASSERT_TRUE(back_or_forward(b,c));
    ASSERT_TRUE(back_or_forward(c,a));
}

void render_line(std::vector<Eigen::RowVector2i> &tri,
                 std::vector<Eigen::RowVector2i> picture) {
    // line只有两个点，
    if (tri.size() == 2) {
        if (tri[0][1] > tri[1][1]) std::swap(tri[0], tri[1]); // 确定了y大的在后面
        // 之后应该怎么做呢？
        for (float i = 0; i < 1; i += 0.01) {
            for (float j = 0; j < 1; j += 0.01) {
                // 高点x值大
                // y = y_0 + (y_1 - y_0) * i;
                // x = x_0 + (x_1 - x_0) * i;
                //给x,y位置的点添加颜色
                // 高点x值小 // 总有一个是需要递减的
                // y = y_1 + (y_0 - y_1) * i;
                // x = x_0 + (x_1 - x_0) * i;
                // 教授的代码上也是这样完成的，之后的问题斜率，原代码中是怎么通过斜率来高效的计算的。
                // 其实两个都还没有搞定，三角形也好，直线也好

            }
        }
    }
}

void render_triangle(std::vector<Eigen::RowVector2i> &tri,
                     std::vector<Eigen::RowVector2i> picture) {
    //第一步，按照y从小到达排列，目前绘制的是平面的三角形，如果是三维世界中的三角形呢？
    //需要先进行透视变换，变成平面的，
    if (tri.size() == 3) {
        if (tri[0][1] > tri[1][1]) std::swap(tri[0], tri[1]);
        if (tri[1][1] > tri[2][1]) std::swap(tri[1], tri[2]);
        if (tri[0][1] > tri[1][1]) std::swap(tri[0], tri[1]);
        // 0 的位置存储a 1的位置存储b 2 的位置存储c
        // 0 的位置存储a 1的位置存储c 2 的位置存储b    acb  abc
        // 0 的位置存储b 1的位置存储a 2 的位置存储c    abc
        // 0 的位置存储b 1的位置存储c 2 的位置存储a    bca  bac  abc
        // 0 的位置存储c 1的位置存储b 2 的位置存储a    bca  bac  abc
        // 0 的位置存储c 1的位置存储a 2 的位置存储b    acb  abc
        //我写的这个版本是可以的，最小的在最开始，最大的在最后面，先确定最大的，

        // 还有另一个版本，先确定最小的，两种不一样的思路，重要的都是比较的顺序的递进
        // if (tri[0][1] > tri[1][1]) std::swap(tri[0], tri[1]); //大的向后移动
        // if (tri[0][1] > tri[2][1]) std::swap(tri[0], tri[2]); //最大的是0
        // if (tri[1][1] > tri[2][1]) std::swap(tri[0], tri[1]); // 中间的和最小的相比较

        // 按照大小排序好之后赢过怎么办呢？目前只是值的了中间的y值点，但是x的顺序是不知道的。
        // 从y的低点到中间值，一直计算左右两边的x值，然后左右两边的x值比较后再从左向右填充
        // 然后再按照台阶向上

        // 之后是优化部分的内容，如何把上下两部分的三角形合并成一个，为什么要分成两个，因为需要单调
        // 或者说for是单调的，暂时先这个样子，后续再说。
    }
}
