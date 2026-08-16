//
// Created by 潘鑫 on 2026/8/10.
//

#ifndef HELLO_MAC_COX_DE_BOOR_H
#define HELLO_MAC_COX_DE_BOOR_H
#include <vector>
#include <Eigen/Dense>
#include <algorithm>


/**
 * 【完全直接代数求解版】
 * 无任何中间插值步，直接输入参数 t 和 7 个节点，独立计算 4 个控制点的多项式系数
 * 传入的 u 包含：u_{k-2}, u_{k-1}, u_k, u_{k+1}, u_{k+2}, u_{k+3}, u_{k+4}
 */
template<typename T>
std::array<double, 4> calculate4PointCoefficientsDirect(float t, const std::array<double, 7> &u) {
    auto safe_div = [](float num, float denom) -> float {
        return (std::abs(denom) > 1e-9f) ? (num / denom) : 0.0f;
    };

    // 因为固定是7个节点,所以默认 第三个 是 1, 和 本质上 还是和 degree 有关

    // 1. 将 7 个节点对齐到 De Boor 的标准局部节点符号
    // K1 = u[0], K2 = u[1], K3 = u[2] (即 u_k), K4 = u[3] (即 u_{k+1}), K5 = u[4], K6 = u[5]
    float K1 = u[0]; // u_{k-2}
    float K2 = u[1]; // u_{k-1}
    float K3 = u[2]; // u_k      (当前区间左端)
    float K4 = u[3]; // u_{k+1}  (当前区间右端)
    float K5 = u[4]; // u_{k+2}
    float K6 = u[5]; // u_{k+3}


    // 2. 预先算出三层金字塔所有的局部线性代数比例项 (这些仅仅是关于 t 的一阶标量函数)
    float a1 = safe_div(t - K1, K4 - K1);
    float a2 = safe_div(t - K2, K5 - K2);
    float a3 = safe_div(t - K3, K6 - K3);

    float b2 = safe_div(t - K2, K4 - K2);
    float b3 = safe_div(t - K3, K5 - K3);

    float c3 = safe_div(t - K3, K4 - K3);

    std::array<double, 4> coeffs;

    // 3. 【核心代数解析式】完全不经过中间控制点插值，各点系数相互独立、直接展开

    // --- 原始点 P0 (controlPoints[k-3]) 的直接代数多项式 ---
    coeffs[0] = (1.0f - c3) * (1.0f - b2) * (1.0f - a1);

    // --- 原始点 P1 (controlPoints[k-2]) 的直接代数多项式 ---
    coeffs[1] = (1.0f - c3) * (1.0f - b2) * a1 +
                (1.0f - c3) * b2 * (1.0f - a2) +
                c3 * (1.0f - b3) * (1.0f - a2);

    // --- 原始点 P2 (controlPoints[k-1]) 的直接代数多项式 ---
    coeffs[2] = (1.0f - c3) * b2 * a2 +
                c3 * (1.0f - b3) * a2 +
                c3 * b3 * (1.0f - a3);

    // --- 原始点 P3 (controlPoints[k]) 的直接代数多项式 ---
    coeffs[3] = c3 * b3 * a3;

    // 4. 消除浮点数极微小的精度截断误差
    float sum = coeffs[0] + coeffs[1] + coeffs[2] + coeffs[3];
    if (std::abs(sum - 1.0f) > 1e-6f) {
        coeffs[0] /= sum;
        coeffs[1] /= sum;
        coeffs[2] /= sum;
        coeffs[3] /= sum;
    }

    return coeffs;
}


/**
 * 【通用直接代数求解版】
 * 无任何控制点插值步，直接输入参数 t 和激活的局部节点片段，独立计算所有非零控制点的多项式系数（基函数值）。
 *
 * @param t 当前参数值
 * @param knot_index 当前 t 所在的激活节点区间左端点索引，满足 t \in [knots[knot_index], knots[knot_index+1])
 * @param knots 全局节点矢量（或者至少包含计算所需的局部节点片段）
 * @param degree 样条次数（如：线性=1, 二次=2, 三次=3...）
 * @return 返回一个大小为 degree + 1 的 std::vector<double>，表示对应控制点的直接组合系数
 */
std::vector<double> calculatePointCoefficientsDirect(double t,
                                                     int knot_index,
                                                     const std::vector<double> &knots,
                                                     int degree = 3) {
    // 分配 degree + 1 个系数空间（对应原先的 coeffs 数组）
    std::vector<double> coeffs(degree + 1, 0.0);

    // 基础防御性处理：0次样条或异常
    if (degree < 0) return coeffs;

    // 1. 初始化第 0 次（阶）基函数：在当前激活区间内，只有当前位置的基函数值为 1.0
    coeffs[degree] = 1.0;

    // 临时数组，用于在递推过程中存储左侧和右侧的节点距离，避免重复计算
    std::vector<double> left(degree + 1, 0.0);
    std::vector<double> right(degree + 1, 0.0);

    // 2. 使用 Cox-de Boor 递推公式动态织网，直接生成代数多项式系数
    // 外层循环 j 代表当前正在计算的次数（从 1 次方一直递推到指定的 degree 次方）
    for (int j = 1; j <= degree; ++j) {
        left[j]  = t - knots[knot_index + 1 - j];
        right[j] = knots[knot_index + j] - t;

        double saved = 0.0;

        // 内层循环利用动态规划，把上一次方（j-1）的系数加权融合到当前次方（j）
        for (int r = 0; r < j; ++r) {
            double denominator = right[r + 1] + left[j - r];

            if (denominator > 1e-9) {
                double term = coeffs[degree - r] / denominator;
                // 这一步对应你原代码里类似 (1.0f - c3) 或 c3 这样的代数分配
                coeffs[degree - r] = saved + right[r + 1] * term;
                saved              = left[j - r] * term;
            } else {
                coeffs[degree - r] = saved;
                saved              = 0.0;
            }
        }
        coeffs[degree - j] = saved;
    }

    // 3. 消除浮点数极微小的精度截断误差归一化（等价于原代码的第 4 步）
    double sum = 0.0;
    for (double c: coeffs) sum += c;
    if (std::abs(sum - 1.0) > 1e-6) {
        for (double &c: coeffs) {
            c /= sum;
        }
    }

    return coeffs;
}


template<typename T>
std::array<double, 4> calculate4PointCoefficients(double t, const std::array<double, 7> &u) {
    // 第一层插值权重（4变3）
    double a1 = (t - u[0]) / (u[3] - u[0]); // (t - u_{k-2}) / (u_{k+1} - u_{k-2})
    double a2 = (t - u[1]) / (u[4] - u[1]); // (t - u_{k-1}) / (u_{k+2} - u_{k-1})
    double a3 = (t - u[2]) / (u[5] - u[2]); // (t - u_k)     / (u_{k+3} - u_k)

    // 第二层插值权重（3变2）
    double b1 = (t - u[1]) / (u[3] - u[1]); // (t - u_{k-1}) / (u_{k+1} - u_{k-1})
    double b2 = (t - u[2]) / (u[4] - u[2]); // (t - u_k)     / (u_{k+2} - u_k)

    // 第三层插值权重（2变1）
    double c1 = (t - u[2]) / (u[3] - u[2]); // (t - u_k)     / (u_{k+1} - u_k)

    // 逆向级联组合（把三层线性插值像金字塔一样剥开合并）
    // 最终曲线点 Pt = c1 * Layer2[1] + (1 - c1) * Layer2[0]
    // 依次展开后，直接得到 4 个原始控制点的贡献系数：
    std::array<double, 4> coeffs;

    coeffs[0] = (1.0 - c1) * (1.0 - b1) * (1.0 - a1);
    coeffs[1] = (1.0 - c1) * (1.0 - b1) * a1 + (1.0 - c1) * b1 * (1.0 - a2) + c1 * (1.0 - b2) * (1.0 - a2);
    coeffs[2] = (1.0 - c1) * b1 * a2 + c1 * (1.0 - b2) * a2 + c1 * b2 * (1.0 - a3);
    coeffs[3] = c1 * b2 * a3;

    return coeffs;
}

/**
* 【已修正】De Boor 算法：计算特定参数 t 对应的单个曲线点坐标
*/
template<typename T>
T deBoor(const double t, const std::vector<double> &knots, const std::vector<T> &controlPoints, int degree = 3) {
    const int n    = static_cast<int>(controlPoints.size()) - 1;
    int knot_index = 0;

    // 1. 寻找 t 所在的激活节点区间 [knots[k], knots[k+1])
    if (t >= knots[n + 1]) {
        knot_index = n; // 处理边界 t = 1.0 的情况
    } else {
        auto it    = std::upper_bound(knots.begin(), knots.end(), t);
        knot_index = static_cast<int>(std::distance(knots.begin(), it)) - 1;
    }
    // k 指的是 当前在那个 knots 的 区间中

    // 2. 提取当前区间相关的 p + 1 个控制点
    std::vector<T> relation_cp;
    relation_cp.reserve(degree + 1);
    for (int j = knot_index - degree; j <= knot_index; ++j) {
        relation_cp.push_back(controlPoints[j]);
    }
    std::array<double, 7> u_7pts = {
        knots[knot_index - 2],
        knots[knot_index - 1],
        knots[knot_index],
        knots[knot_index + 1],
        knots[knot_index + 2],
        knots[knot_index + 3],
        knots[knot_index + 4]
    };

    auto coeffs = calculate4PointCoefficientsDirect<T>(t, u_7pts);

    T final_point = relation_cp[0] * coeffs[0] +
                    relation_cp[1] * coeffs[1] +
                    relation_cp[2] * coeffs[2] +
                    relation_cp[3] * coeffs[3];

    return final_point;

    // 3. 迭代线性插值
    for (int r = 1; r <= degree; ++r) {
        for (int j = degree; j >= r; --j) {
            // 【核心修正点】：映射到全局控制点的实际索引 i
            int i = j + knot_index - degree;

            // 计算当前层级的节点分母
            double denom = knots[i + degree + 1 - r] - knots[i];
            double alpha = 0.0;
            if (denom > 1e-9) {
                alpha = (t - knots[i]) / denom;
            }

            // 执行插值
            relation_cp[j].x() = (1.0 - alpha) * relation_cp[j - 1].x() + alpha * relation_cp[j].x();
            relation_cp[j].y() = (1.0 - alpha) * relation_cp[j - 1].y() + alpha * relation_cp[j].y();
        }
    }

    return relation_cp[degree];
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

#endif //HELLO_MAC_COX_DE_BOOR_H
