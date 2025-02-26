//
// Created by 潘鑫 on 2025/2/24.
//
//既然是曲线之间的规划，先说两点之间吧

#include "gtest/gtest.h"
#include <vector>
/**
 * 既然是曲线之间的规划，先说两点之间吧
 * 位置等于速度✖️时间，
 * p = vt + p0
 * v = at + v0
 * p = p0 + v0t + att
 * 给出初始位置，结束位置，初
 * 始速度大小和方向，结束位置大小和方向，
 * 最大加速度和最大减速度
 * 最大速度 v_max = 5
 *  p_s = 0  , p_f  = 10  , v_s = v_f = 0 , a = 3
 *  两个阶段，加速阶段，
 *  最大加速度是能过直接到达还是缓慢到达
 *  这里只需要返回a和方向
 */
void test_curve() {
}


/**
 *
 * @param start
 * @param end
 * @param number number_between_start_and_end
 * @param numbers_vector
 */
template<typename T>
void direct_connect_two_point(T start, T end, int number, std::vector<T> &numbers_vector) {
    // 没有重复添加起始点和终止点
    numbers_vector.push_back(start);
    for (int i = 1; i < number; ++i) {
        T tem_point = i * (end - start) / number + start;
        numbers_vector.push_back(tem_point);
    }
    numbers_vector.push_back(end);
    // 最后返回没有,无所谓了，改为引用vector 添加 节点
    // 然后又改成了使用模版的函数
}

template<typename T>
void direct_connect_two_point_fast(T start, T end, int number, int max_speed, int max_accelerate,
                                   std::vector<T> &numbers_vector) {
    // 没有重复添加起始点和终止点
    numbers_vector.push_back(start);
    T length = end - start;
    int accelerate_time = max_speed / max_accelerate;
    // 如果长度大于匀加速和匀减速阶段 ，那么中间是有一段时间的最高速度的
    // 匀加速的距离 为 最大速度 ✖️ 时间 除于 2
    if (length > accelerate_time * max_speed) {
        int total_time = (length - accelerate_time * max_speed) / max_speed + accelerate_time * 2;
        int current_speed = 0;
        T current_position = start;
        float time_step = total_time / number;
        // 加速阶段
        for (int i = 1; i < accelerate_time / total_time * number; ++i) {
            T current_position = current_position +
                                 current_speed * time_step +
                                 1 / 2 * max_accelerate * time_step * time_step;
            current_speed = current_speed + max_accelerate * time_step;
            numbers_vector.push_back(start);
            numbers_vector.push_back(current_position);
        }
        // 匀速阶段


        // 减速阶段
    } else {
        // 否则中间是达不到最大速度的
        // 达不到最大速度，那就需要计算出最大速度
        // 距离已知，求时间
        // x_middle = 1/2 * att
        // t = sqrt( x/a )
    }
    // 还什么不足之处呢？方向，都是在单个方向上运行
}


/**
 * 这个问题描述的估计都有点问题，
 * @tparam T
 * @param start
 * @param end
 * @param start_speed
 * @param end_speed
 * @param number
 * @param numbers_vector
 */
template<typename T>
void direct_connect_two_point_with_speed(T start, T end,
                                         T start_speed, T end_speed,
                                         int number, std::vector<T> &numbers_vector) {
    // 没有重复添加起始点和终止点
    // 添加速度，没有速度的限制，应该先把这个做掉的。
    numbers_vector.push_back(start);
    T speed_error = (end_speed - start_speed);
    T length = end - start; // 但是还给定的最后的位置，最后可能不在想要的位置

    T time = length / (start_speed + (1 / 2) * speed_error); //这样算出来的时间是不是一致的？

    T speed_step = (end_speed - start_speed) / time; // 这里是每步速度的变化量

    // length = current_speed * time + (1 / 2) * speed_step * time * time
    // length = ( speed + (1/2)speed_error) time // 这里可以求出time
    //
    // position = speed * time + (1/2)att
    // at = speed_error
    // position =( speed + (1/2)speed_error) time

    T current_speed = start_speed;
    for (int i = 1; i < number; ++i) {
        const int time = 1;
        T tem_point = current_speed * time + start + (1 / 2) * speed_step * time * time;
        numbers_vector.push_back(tem_point);
    }
    numbers_vector.push_back(end);
    // 最后返回没有,无所谓了，改为引用vector 添加 节点
    // 然后又改成了使用模版的函数
}


//两个点之间的一条曲线，目的是能过计算出连续的线，通过给定起始点和结束的速度的大小和方向，
// 最后求出整条路径。  // 通过给定起始点和结束的速度的大小和方向 这句话有问题
// 给定起始点的位置和速度，计算如何快捷的到底下一个点和下下个点，规划好如何按照顺序通过所有的点，
// 再之后还需要给定赛道，需要不碰撞的情况下

//先按照汽车的模型开，角度与速度的是有一个圆的半径的，这也是一个很有趣的问题。
// 先定匀速，然后有赛车的转弯角度， 降速 ，看什么时候能刚好切过去，
// 什么算刚好切过去呢？ 几何问题，已经知道怎么算了，但是没有写出来最终的结果。

// 这里来想办法贝塞尔曲线给做出来吧，
// 其实我记得还有另外的曲线的，但是不知道在哪里找到了。   // Hermite插值   Kochanek-Bartels曲线  B-样条 多项式曲线
// 贝塞尔曲线的 position =  (1-a)x + ay  a 属于 0到1
// 二阶是这个样子吗？ 它的系数分别是 ((1-a) + a)((1-a) + a) 他们几个的组合
// 三阶 ((1-a) + a)((1-a) + a)((1-a) + a)
// 四阶 ((1-a) + a)((1-a) + a)((1-a) + a)((1-a) + a)

// 为什么是对的需要看这两个网页 https://en.wikipedia.org/wiki/Binomial_theorem (二项式系数)
// 和 https://pomax.github.io/bezierinfo/zh-CN/index.html

// 矩阵形式确实很好看，但是如果你把三维坐标带入P中，那么是需要按照三维的轴对齐来相加得到最后的结果，
// 也就是需要在前面加一个长度为四的单位行向量，还需要正确添加括号

// 贝塞尔曲线重要的是它是怎么递推的，
// B样条函数重要的也是它是如何递推的

// 想办法把 矩阵库引入过来
#include "Eigen/Eigen"
//然后按照矩阵的形式把结果算出来。


bool bezier_cerve(std::vector<Eigen::Vector2d> &positions, Eigen::Vector2d &new_position, float u) {
    if (positions.size() == 4) {
        Eigen::Matrix4d m{{1, 0, 0, 0}, {-3, 3, 0, 0}, {3, -6, 3, 0}, {-1, 3, -3, 1}};
        Eigen::RowVector4d u_vector{1, u, u * u, u * u * u};
        Eigen::Matrix<double, 4, 2> n;
        n << positions[0], positions[1], positions[2], positions[3];
        new_position = (u_vector * m * n).transpose();
        return true;
    } else return false;
}

TEST(bezier,curve) {
    std::vector<Eigen::Vector2d> positions;
    positions.push_back(Eigen::Vector2d(0, 0));
    positions.push_back(Eigen::Vector2d(0, 1));
    positions.push_back(Eigen::Vector2d(1, 0));
    positions.push_back(Eigen::Vector2d(1, 2));
    Eigen::Vector2d new_position;
    bezier_cerve(positions, new_position, 0.1);
    std::cout << new_position << std::endl;
    bezier_cerve(positions, new_position, 0.5);
    std::cout << new_position << std::endl;
    bezier_cerve(positions, new_position, 0.75);
    std::cout << new_position << std::endl;
}
