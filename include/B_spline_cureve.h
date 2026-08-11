//
// Created by 潘鑫 on 2026/8/10.
//

#ifndef HELLO_MAC_SPLINE_CUREVE_H
#define HELLO_MAC_SPLINE_CUREVE_H
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include "bezier_curve.h"

template<typename T>
class B_spline {
public:
    // 计算点 M 到线段 AB 的垂直距离（弦高）
    double getSegmentDistance(const T &M, const T &A, const T &B) {
        double dx    = B.x - A.x;
        double dy    = B.y - A.y;
        double lenSq = dx * dx + dy * dy;

        // 如果 A 和 B 几乎重合，直接返回 M 到 A 的距离
        if (lenSq < 1e-12) {
            return std::sqrt((M.x - A.x) * (M.x - A.x) + (M.y - A.y) * (M.y - A.y));
        }

        // 点到直线的距离公式
        double num = std::abs(dy * M.x - dx * M.y + B.x * A.y - B.y * A.x);
        return num / std::sqrt(lenSq);
    }


    /**
     * 2. 自动生成普通准均匀（Clamped）B样条的节点向量
     * 中间节点均匀等距且不重复
     */
    static std::vector<double> generateClampedKnots(size_t numControlPoints, int degree = 3) {
        int p           = degree;
        int n           = static_cast<int>(numControlPoints) - 1;
        int numInternal = n - p;

        // if (numInternal < 0) {
        //     throw std::invalid_error("控制点数量必须大于等于曲线阶数加1（对于三次曲线，至少需要4个点）");
        // }

        std::vector<double> knots;
        knots.reserve(numControlPoints + p + 1);

        // 头部重复 p + 1 次
        for (int i = 0; i <= p; ++i) {
            knots.push_back(0.0);
        }

        // 中间节点均匀分布（不重复）
        if (numInternal > 0) {
            // 总段数为 numInternal + 1
            double step = 1.0 / (numInternal + 1);
            for (int i = 1; i <= numInternal; ++i) {
                knots.push_back(i * step);
            }
        }

        // 尾部重复 p + 1 次
        for (int i = 0; i <= p; ++i) {
            knots.push_back(1.0);
        }

        return knots;
    }

    /**
     * 3. De Boor 算法：计算特定参数 t 对应的单个曲线点坐标
     */

    /**
 * 【已修正】De Boor 算法：计算特定参数 t 对应的单个曲线点坐标
 */
    static T deBoor(double t, const std::vector<double> &knots, const std::vector<T> &controlPoints, int degree = 3) {
        int p = degree;
        int n = static_cast<int>(controlPoints.size()) - 1;
        int k = 0;

        // 1. 寻找 t 所在的激活节点区间 [knots[k], knots[k+1])
        if (t >= knots[n + 1]) {
            k = n; // 处理边界 t = 1.0 的情况
        } else {
            auto it = std::upper_bound(knots.begin(), knots.end(), t);
            k       = static_cast<int>(std::distance(knots.begin(), it)) - 1;
        }

        // 2. 提取当前区间相关的 p + 1 个控制点
        std::vector<T> d;
        d.reserve(p + 1);
        for (int j = k - p; j <= k; ++j) {
            d.push_back(controlPoints[j]);
        }

        // 3. 迭代线性插值
        for (int r = 1; r <= p; ++r) {
            for (int j = p; j >= r; --j) {
                // 【核心修正点】：映射到全局控制点的实际索引 i
                int i = j + k - p;

                // 计算当前层级的节点分母
                double denom = knots[i + p + 1 - r] - knots[i];
                double alpha = 0.0;
                if (denom > 1e-9) {
                    alpha = (t - knots[i]) / denom;
                }

                // 执行插值
                d[j].x() = (1.0 - alpha) * d[j - 1].x() + alpha * d[j].x();
                d[j].y() = (1.0 - alpha) * d[j - 1].y() + alpha * d[j].y();
            }
        }

        return d[p];
    }


    // static T deBoor(double t, const std::vector<double> &knots, const std::vector<T> &controlPoints,
    //                 int degree = 3) {
    //     int p = degree;
    //     int k = 0;
    //     int n = static_cast<int>(controlPoints.size()) - 1;
    //
    //     // 处理边界 t = 1.0 的特殊情况
    //     if (std::abs(t - knots.back()) < 1e-9) {
    //         k = n;
    //     } else {
    //         // 寻找 t 所在的节点区间 [knots[k], knots[k+1])
    //         auto it = std::upper_bound(knots.begin(), knots.end(), t);
    //         k       = static_cast<int>(std::distance(knots.begin(), it)) - 1;
    //     }
    //
    //     // 提取当前区间相关的 p + 1 个控制点
    //     std::vector<T> d;
    //     d.reserve(p + 1);
    //     for (int j = k - p; j <= k; ++j) {
    //         d.push_back(controlPoints[j]);
    //     }
    //
    //     // 迭代线性插值
    //     for (int r = 1; r <= p; ++r) {
    //         for (int j = p; j >= r; --j) {
    //             double denom = knots[j + k - p + r] - knots[j + k - p];
    //             double alpha = 0.0;
    //             if (denom > 1e-9) {
    //                 alpha = (t - knots[j + k - p]) / denom;
    //             }
    //
    //             // 对 X 和 Y 坐标分别进行线性插值
    //             d[j].x() = (1.0 - alpha) * d[j - 1].x() + alpha * d[j].x();
    //             d[j].y() = (1.0 - alpha) * d[j - 1].y() + alpha * d[j].y();
    //         }
    //     }
    //
    //     return d[p];
    // }


    /**
     * 自适应细分核心递归函数
     */
    static void sampleAdaptive(double t_a, const T &A, double t_b, const T &B,
                               double tess_tol, int max_depth, int current_depth,
                               const std::vector<double> &knots,
                               const std::vector<T> &controlPoints,
                               int degree,
                               std::vector<T> &out_path) {
        // 1. 计算中点参数及对应的曲线坐标
        double t_m = 0.5 * (t_a + t_b);
        T M        = deBoor(t_m, knots, controlPoints, degree);

        // 3. 判别是否满足精度要求，或者达到了最大递归深度（防止死循环）
        if (!need_tessellation(A, M, B, tess_tol) || current_depth >= max_depth) {
            // 满足精度，将终点 B 压入路径（起点 A 会由上一段或者最开始压入）
            out_path.push_back(B);
        } else {
            // 不满足精度，分别对左半段和右半段进行递归细分
            sampleAdaptive(t_a, A, t_m, M, tess_tol, max_depth, current_depth + 1, knots, controlPoints, degree,
                           out_path);
            sampleAdaptive(t_m, M, t_b, B, tess_tol, max_depth, current_depth + 1, knots, controlPoints, degree,
                           out_path);
        }
    }

    /**
 * 修改后的自适应细分函数
 */
    static void sampleAdaptiveWithFlatness(double t_a, const T &p1, double t_b, const T &p4,
                                           float tess_tol, int max_depth, int current_depth,
                                           const std::vector<double> &knots, const std::vector<T> &controlPoints,
                                           int degree,
                                           std::vector<T> &out_path) {
        // 1. 如果到了最大深度，强制终止，避免堆栈溢出
        if (current_depth >= max_depth) {
            out_path.push_back(p4);
            return;
        }

        // 2. 在当前区间内，通过三等分采样，获得中间的两个探测点 p2 和 p3
        double delta    = t_b - t_a;
        double t_m1     = t_a + delta / 3.0;
        double t_m2     = t_a + 2.0 * delta / 3.0;
        double t_center = t_a + 0.5 * delta;

        T p2 = deBoor(t_m1, knots, controlPoints, degree);
        T p3 = deBoor(t_m2, knots, controlPoints, degree);

        // 3. 调用你的高效率检验算法
        if (!need_tessellation(p1, p2, p3, p4, tess_tol)) {
            // 如果足够平直，直接收尾（压入端点 p4）
            out_path.push_back(p4);
        } else {
            // 如果不够平直，以中点为界，递归二分
            T p_center = deBoor(t_center, knots, controlPoints, degree);

            // 左半段递归：从 t_a 到 t_center
            sampleAdaptiveWithFlatness(t_a, p1, t_center, p_center, tess_tol, max_depth, current_depth + 1, knots,
                                       controlPoints, degree, out_path);
            // 右半段递归：从 t_center 到 t_b
            sampleAdaptiveWithFlatness(t_center, p_center, t_b, p4, tess_tol, max_depth, current_depth + 1, knots,
                                       controlPoints, degree, out_path);
        }
    }


    /**
     * 4. 主调用函数：计算整条 B 样条路径上的所有离散点
     */
    std::vector<T> calculateBSplinePath(const std::vector<T> &controlPoints,
                                        int numSamples = 100,
                                        int degree     = 3) {
        // 自动生成节点向量
        std::vector<double> knots = generateClampedKnots(controlPoints.size(), degree);
        std::vector<T> path;
        path.reserve(numSamples);

        // 在 [0.0, 1.0] 范围内进行步长采样
        double step = 1.0 / (numSamples - 1);
        for (int i = 0; i < numSamples; ++i) {
            double t = i * step;
            // 防止浮点数微小误差超出 1.0
            if (t > 1.0) t = 1.0;

            T pt = deBoor(t, knots, controlPoints, degree);
            path.push_back(pt);
        }

        return path;
    }

    /**
     * 根据 tess_tol 计算 B 样条路径的主入口函数
     */
    static std::vector<T> calculateBSplinePathWithTol(const std::vector<T> &controlPoints, double tess_tol,
                                                      int degree = 3) {
        std::vector<double> knots = generateClampedKnots(controlPoints.size(), degree);
        std::vector<T> path;

        if (controlPoints.empty()) return path;

        // 计算起点 (t=0.0) 和 终点 (t=1.0)
        T startPt = deBoor(0.0, knots, controlPoints, degree);
        T endPt   = deBoor(1.0, knots, controlPoints, degree);

        // 压入起始点
        path.push_back(startPt);

        // 限制最大递归深度为 10（单段最多细分 1024 次），防止极极端情况造成的堆栈溢出
        int max_depth = 10;

        // 开始自适应递归采样
        sampleAdaptiveWithFlatness(0.0, startPt, 1.0, endPt,
                                   tess_tol, max_depth, 0, knots, controlPoints, degree, path);

        return path;
    }
};

#endif //HELLO_MAC_SPLINE_CUREVE_H
