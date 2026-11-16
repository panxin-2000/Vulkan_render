//
// Created by 潘鑫 on 2026/8/10.
//

#ifndef HELLO_MAC_SPLINE_CUREVE_H
#define HELLO_MAC_SPLINE_CUREVE_H
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include "Cox-de_Boor.h"
#include "bezier_curve.h"

template<typename T>
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
inline std::vector<double> generateClampedKnots(size_t numControlPoints, int degree = 3) {
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
 * 自适应细分核心递归函数
 */
template<typename T>
void sampleAdaptive(double t_a, const T &A, double t_b, const T &B,
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
template<typename T>
void sampleAdaptiveWithFlatness(double t_a, const T &p1, double t_b, const T &p4,
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
template<typename T>
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
template<typename T>
std::vector<T> calculateBSplinePathWithTol(const std::vector<T> &controlPoints,
                                           const std::vector<double> &knots,
                                           double tess_tol,
                                           int degree = 3) {
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


template<typename T>
class B_spline {
    std::vector<T> points_;
    std::vector<double> knots;
    int degree;

public:
    void add_point(T x) {
        points_.push_back(x);
    }

    void push_back(T x) {
        points_.push_back(x);
    }

    bool insert_point(T x, size_t index) {
        if (points_.size() <= index) {
            return false;
        }
        points_.insert(points_.begin() + index, x);
        return true;
    }

    bool remove_point(T x) {
        std::erase(points_, x);
        return true;
    }

    bool remove_point(size_t index) {
        points_.erase(points_.begin() + index);
        return true;
    }

    auto get_path(double tess_tol = 1.25) {
        std::vector<double> knots = generateClampedKnots(points_.size(), 3);
        return calculateBSplinePathWithTol<T>(points_, knots, tess_tol, 3);
    }

    T EvaluateDerivative(double u) const {
        int n = static_cast<int>(points_.size()) - 1;
        int p = degree;

        // 1. 安全边界检查：使用 Eigen 特有的静态零构造
        if (n < p || knots.size() != static_cast<size_t>(n + p + 2)) {
            return T::Zero();
        }

        // 2. 钳制参数 u
        double u_min = knots[p];
        double u_max = knots[n + 1];
        if (u < u_min) u = u_min;
        if (u > u_max) u = u_max;

        int k = p;
        if (std::abs(u - u_max) < 1e-7) {
            k = n;
        } else {
            auto it = std::upper_bound(knots.begin() + p, knots.end() - p, u);
            k       = static_cast<int>(std::distance(knots.begin(), it)) - 1;
        }

        // 3. 准备局部缓冲区
        // 为了防止 Eigen 的单精度 float 在多轮递推中产生舍入误差，
        // 我们在内部将控制点临时转换为全双精度（double）的 Eigen 向量进行计算
        using TD = typename Eigen::Matrix<double, T::RowsAtCompileTime, T::ColsAtCompileTime>;
        std::vector<TD> d_points(p);

        for (int i = 0; i < p; ++i) {
            int idx      = k - p + 1 + i;
            double denom = knots[idx + p] - knots[idx];

            if (denom > 1e-7) {
                double factor = static_cast<double>(p) / denom;
                // 使用 .template cast<double>() 将 Vector2f 安全转为 Vector2d 参与高精度计算
                d_points[i] = (points_[idx].template cast<double>() - points_[idx - 1].template cast<double>()) *
                              factor;
            } else {
                d_points[i] = TD::Zero();
            }
        }

        // 4. De Boor 算法高精度递推
        int p_deriv = p - 1;
        for (int r = 1; r <= p_deriv; ++r) {
            for (int i = p_deriv; i >= r; --i) {
                int idx      = k - p_deriv + i;
                double alpha = (u - knots[idx]) / (knots[idx + p_deriv + 1 - r] - knots[idx]);

                d_points[i] = d_points[i - 1] * (1.0 - alpha) + d_points[i] * alpha;
            }
        }

        // 5. 最终将双精度结果重新转回您的原生类型 T (即 Eigen::Vector2f) 输出
        return d_points[p_deriv].template cast<typename T::Scalar>();
    }

    float CalculateSplineLength() {
        double totalLength = 0.0;
        const auto &knots  = generateClampedKnots(points_.size(), 3);

        if (knots.size() < 2) return 0.0f;

        // 外层循环：遍历所有节点区间（Span）
        for (size_t i = 0; i < knots.size() - 1; ++i) {
            double u_start = knots[i];
            double u_end   = knots[i + 1];

            // 跨过重合节点（无实际长度的无效区间）
            if (std::abs(u_end - u_start) < 1e-6) {
                continue;
            }

            // 内层循环：对当前有效区间 [u_start, u_end] 进行高斯积分
            double subLength = 0.0;
            for (int g = 0; g < GAUSS_COUNT; ++g) {
                // 将标准区间 [-1, 1] 映射到当前节点区间 [u_start, u_end]
                // u = 0.5 * (u_end - u_start) * x + 0.5 * (u_start + u_end)
                double u = 0.5 * (u_end - u_start) * GAUSS_X[g] + 0.5 * (u_start + u_end);

                // 计算当前参数 u 处的切向量
                T dP = EvaluateDerivative(static_cast<float>(u));

                // 计算模长（瞬时速度）
                double speed = std::sqrt(static_cast<double>(dP.x) * dP.x + static_cast<double>(dP.y) * dP.y);

                // 高斯加权求和
                subLength += GAUSS_W[g] * speed;
            }

            // 乘以当前区间的缩放因子： (u_end - u_start) / 2
            totalLength += subLength * 0.5 * (u_end - u_start);
        }

        return static_cast<float>(totalLength);
    }

public:
    // 计算点 M 到线段 AB 的垂直距离（弦高）
};

#endif //HELLO_MAC_SPLINE_CUREVE_H
