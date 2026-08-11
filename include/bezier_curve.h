//
// Created by 潘鑫 on 2026/8/10.
//

#ifndef HELLO_MAC_BEZIER_CURVE_H
#define HELLO_MAC_BEZIER_CURVE_H
#include <Eigen/Dense>
#include <vector>


template<typename T>
void PathBezierCubicCurveToCasteljau(std::vector<T> *path, const T &p1, const T &p2, const T &p3, const T &p4,
                                     float tess_tol,
                                     const int level) {
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


template<typename T>
class Bezier {
public:
    Bezier(const T &p1, const T &p2, const T &p3, const T &p4,
           float tess_tol) : p1_(p1), p2_(p2), p3_(p3), p4_(p4), tess_tol_(tess_tol) {
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
