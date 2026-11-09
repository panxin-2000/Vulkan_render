//
// Created by 潘鑫 on 2026/8/10.
//

#ifndef HELLO_MAC_BEZIER_CURVE_H
#define HELLO_MAC_BEZIER_CURVE_H
#include <Eigen/Dense>
#include <vector>


template<typename T>
bool need_tessellation(const T &p1, const T &mid, const T &p4, float tess_tol) {
    // 1. 计算终点到起点的位移向量
    T d41 = p4 - p1;

    // 2. 利用二维向量叉乘，计算中点 mid 偏离端点连线（d41）的几何关系
    // mid - p4 是中点到终点的向量，它与 d41 的叉乘绝对值代表平行四边形面积
    float d_mid = std::abs((mid - p4).cross(d41));

    // 3. 误差平直度检查 (Flatness test)
    // 原理同四点法，两边平方以干掉开方计算。
    // 因为只有一个中间点，所以原本的 (d2 + d3) 变成了 (d_mid + d_mid) = 2 * d_mid
    // 平方后即为 4 * d_mid * d_mid
    if (4.0f * d_mid * d_mid < tess_tol * d41.squaredNorm()) {
        return false; // 足够平直，不需要细分
    }
    return true; // 不够平直，需要继续细分
}


template<typename T>
bool need_tessellation(const T &p1, const T &p2, const T &p3, const T &p4, float tess_tol) {
    // 1. 计算终点到起点的位移向量
    T d41 = p4 - p1;
    // tess_tol 与像素相关的一个参数

    // 2. 利用二维向量叉乘（Cross Product）计算控制点到端点连线的距离关系
    // Eigen 的 cross 在 2D 中返回标量：a.x() * b.y() - a.y() * b.x()
    float d2 = std::abs((p2 - p4).cross(d41));
    float d3 = std::abs((p3 - p4).cross(d41));
    // 两个向量的叉乘（Cross Product）的绝对值，代表它们组成的平行四边形的面积

    // 3. 误差平直度检查 (Flatness test) // 中间的控制点，偏离“起点到终点的连线”有多远
    if ((d2 + d3) * (d2 + d3) < tess_tol * d41.squaredNorm()) {
        return false;
    }
    return true;
}


template<typename T>
void PathBezierCubicCurveToCasteljau(std::vector<T> *path, const T &p1, const T &p2, const T &p3, const T &p4,
                                     float tess_tol,
                                     const int level) {
    // 3. 误差平直度检查 (Flatness test) // 中间的控制点，偏离“起点到终点的连线”有多远
    if (!need_tessellation(p1, p2, p3, p4, tess_tol)) {
        path->push_back(p4);
    }
    // 4. 递归细分
    else if (level < 10) {
        // 利用 Eigen 向量加权，直接计算中点
        T p12   = (p1 + p2) * 0.5;
        T p23   = (p2 + p3) * 0.5;
        T p34   = (p3 + p4) * 0.5;
        T p123  = (p12 + p23) * 0.5;
        T p234  = (p23 + p34) * 0.5;
        T p1234 = (p123 + p234) * 0.5;

        // 左半段曲线递归
        PathBezierCubicCurveToCasteljau(path, p1, p12, p123, p1234, tess_tol, level + 1);
        // 右半段曲线递归
        PathBezierCubicCurveToCasteljau(path, p1234, p234, p34, p4, tess_tol, level + 1);
    }
}


// 5点高斯-勒让德求积法的常量表 (标准区间为 [-1, 1])
// 采用 double 保证积分中间步骤的计算精度，避免累加过程中的浮点截断误差
constexpr double GAUSS_X[] = {
    -0.9061798459386640, -0.5384693101056831, 0.0000000000000000, 0.5384693101056831, 0.9061798459386640
};
constexpr double GAUSS_W[] = {
    0.2369268850561891, 0.4786286704993665, 0.5688888888888889, 0.4786286704993665, 0.2369268850561891
};
constexpr int GAUSS_COUNT = 5;


template<typename T>
class Bezier {
public:
    Bezier(const T &p1, const T &p2, const T &p3, const T &p4,
           float tess_tol) : p1_(p1), p2_(p2), p3_(p3), p4_(p4), tess_tol_(tess_tol) {
    }


    /**
     * @brief 计算三次贝塞尔曲线在参数 t 处的导数(切向量)
     */
    T GetCubicBezierDerivative(
        const T &p0,
        const T &p1,
        const T &p2,
        const T &p3,
        double t) {
        double u  = 1.0 - t;
        double t2 = t * t;
        double u2 = u * u;
        double ut = u * t;

        T derivative;
        // 使用常引用传递后，通过 .x 和 .y 直接读取
        derivative.x = static_cast<float>(3.0 * u2 * (p1.x - p0.x) + 6.0 * ut * (p2.x - p1.x) + 3.0 * t2 * (
                                              p3.x - p2.x));
        derivative.y = static_cast<float>(3.0 * u2 * (p1.y - p0.y) + 6.0 * ut * (p2.y - p1.y) + 3.0 * t2 * (
                                              p3.y - p2.y));

        return derivative;
    }

    /**
     * @brief 使用高斯-勒让德求积法计算三次贝塞尔曲线的长度
     */
    float CalculateCubicBezierLength(
        const T &p0,
        const T &p1,
        const T &p2,
        const T &p3) {
        double totalLength = 0.0;

        for (int i = 0; i < GAUSS_COUNT; ++i) {
            // 1. 将标准积分区间 [-1, 1] 线性映射到参数区间 [0, 1]
            double t = 0.5 * GAUSS_X[i] + 0.5;

            // 2. 计算当前 t 下的切向量 (dx/dt, dy/dt)
            T dP = GetCubicBezierDerivative(p0, p1, p2, p3, t);

            // 3. 计算速度标量（微元长度速度系数）
            double speed = std::sqrt(static_cast<double>(dP.x) * dP.x + static_cast<double>(dP.y) * dP.y);

            // 4. 累加： 权重 * 函数值
            totalLength += GAUSS_W[i] * speed;
        }

        // 5. 乘以区间缩放因子 0.5，并转回 float 返回
        return static_cast<float>(totalLength * 0.5);
    }


    void Casteljau(std::vector<T> *path, const int level = 0) {
        assert(path != nullptr);
        // 1. 计算终点到起点的位移向量
        T d41 = p4_ - p1_;

        // 2. 利用二维向量叉乘（Cross Product）计算控制点到端点连线的距离关系
        // Eigen 的 cross 在 2D 中返回标量：a.x() * b.y() - a.y() * b.x()
        const float d2 = std::abs((p2_ - p4_).cross(d41));
        const float d3 = std::abs((p3_ - p4_).cross(d41));

        // 3. 误差平直度检查 (Flatness test)
        if ((d2 + d3) * (d2 + d3) < tess_tol_ * d41.squaredNorm()) {
            path->push_back(p4_);
        }
        // 4. 递归细分
        else if (level < 10) {
            // 利用 Eigen 向量加权，直接计算中点
            T p12   = (p1_ + p2_) * 0.5;
            T p23   = (p2_ + p3_) * 0.5;
            T p34   = (p3_ + p4_) * 0.5;
            T p123  = (p12 + p23) * 0.5;
            T p234  = (p23 + p34) * 0.5;
            T p1234 = (p123 + p234) * 0.5;

            PathBezierCubicCurveToCasteljau(path, p1_, p12, p123, p1234, tess_tol_, level + 1);
            PathBezierCubicCurveToCasteljau(path, p1234, p234, p34, p4_, tess_tol_, level + 1);
        }
    }

private:
    // std::array<T, U> point;
    T p1_;
    T p2_;
    T p3_;
    T p4_;
    float tess_tol_;
};


template<typename T>
class Composite_Bezier_Curve {
};


#endif //HELLO_MAC_BEZIER_CURVE_H
