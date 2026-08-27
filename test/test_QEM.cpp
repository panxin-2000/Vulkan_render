//
// Created by 潘鑫 on 2026/8/26.
//


#include <Eigen/Eigen>
#include <gtest/gtest.h>

/**
 * 三个点的顺序要求是逆时针的顺序,得到的是正面的法线
 * @param pA
 * @param pB
 * @param pC
 * @return
 */
Eigen::Vector3f get_normal(const Eigen::Vector3f &pA, const Eigen::Vector3f &pB, const Eigen::Vector3f &pC) {
    Eigen::Vector3f vAB = pB - pA; // 从 A 指向 B
    Eigen::Vector3f vAC = pC - pA; // 从 A 指向 C
    // 3. 计算法线（叉乘）并归一化（单位化）
    // .cross() 表示叉乘，.normalized() 表示将向量长度化为 1
    Eigen::Vector3f normal = vAB.cross(vAC).normalized();
    return normal;
}

Eigen::Vector4f get_plane_equation(const Eigen::Vector3f &pA, const Eigen::Vector3f &pB, const Eigen::Vector3f &pC) {
    // 1. 先求出法线 (A, B, C)
    Eigen::Vector3f vAB    = pB - pA;
    Eigen::Vector3f vAC    = pC - pA;
    Eigen::Vector3f normal = vAB.cross(vAC).normalized();

    // 2. 计算 D 值
    // 因为 Ax + By + Cz + D = 0，所以 D = -(Ax + By + Cz) = -(normal · pA)
    // 这里用点 A、B 或 C 计算结果都一样，我们取 pA
    float D = -normal.dot(pA);

    // 3. 组装成 Vector4f (A, B, C, D) 并返回
    return Eigen::Vector4f(normal.x(), normal.y(), normal.z(), D);
}

Eigen::Matrix4f get_plane_Q_matrix(const Eigen::Vector3f &pA, const Eigen::Vector3f &pB, const Eigen::Vector3f &pC) {
    // 1. 先计算平面方程向量 p = (A, B, C, D)
    Eigen::Vector3f vAB    = pB - pA;
    Eigen::Vector3f vAC    = pC - pA;
    Eigen::Vector3f normal = vAB.cross(vAC).normalized();
    float D                = -normal.dot(pA);

    Eigen::Vector4f p(normal.x(), normal.y(), normal.z(), D);

    // 2. 计算外积 p * p^T 得到 4x4 的 Q 矩阵
    // 在 Eigen 中，列向量乘以其转置会自动得到 Matrix4f
    Eigen::Matrix4f Q = p * p.transpose();

    return Q;
}

// 输入：该顶点相连的所有平面的 Q 矩阵集合
Eigen::Matrix4f compute_vertex_Q(const std::vector<Eigen::Matrix4f> &neighboring_Qs) {
    // 1. 初始化一个全为 0 的 4x4 矩阵
    Eigen::Matrix4f vertex_Q = Eigen::Matrix4f::Zero();
    // 2. 遍历并直接累加
    for (const auto &Q: neighboring_Qs) {
        vertex_Q += Q;
    }
    return vertex_Q;
}


Eigen::Vector3f get_collapsed_vertex(const Eigen::Vector3f &vI, const Eigen::Vector3f &vJ,
                                     const Eigen::Matrix4f &QI, const Eigen::Matrix4f &QJ) {
    Eigen::Matrix4f Q_edge = QI + QJ;

    // 提取 A 矩阵 (3x3) 和 b 向量 (3x1)
    Eigen::Matrix3f A = Q_edge.block<3, 3>(0, 0);
    Eigen::Vector3f b(-Q_edge(0, 3), -Q_edge(1, 3), -Q_edge(2, 3));

    // 使用带有完全主元消元法的 LU 分解来检查可逆性
    Eigen::FullPivLU<Eigen::Matrix3f> lu(A);

    // 如果矩阵可逆，直接求解 Ax = b
    if (lu.isInvertible() && std::abs(A.determinant()) > 1e-5f) {
        return lu.solve(b);
    }

    // 如果退化/不可逆，直接返回中点作为新顶点（代价将在另一个函数中去比较端点和中点）
    return 0.5f * (vI + vJ);
}


// 2. 单独计算某个点在指定 Q 矩阵下的代价 (Cost = v^T * Q * v)
float get_vertex_cost(const Eigen::Vector3f &v, const Eigen::Matrix4f &Q_edge) {
    // 转换为齐次坐标 (x, y, z, 1)
    Eigen::Vector4f v_homo(v.x(), v.y(), v.z(), 1.0f);

    // 函数接口和数据存储用 float，但函数内部乘法用 double 算，最后再转回 float

    // 执行矩阵乘法 v^T * Q * v
    return v_homo.transpose() * Q_edge * v_homo;
}

// QEM 的数学逻辑到这里就结束, 之后就是去计算 half-edge 的 逻辑了

// 1. 绝对代价阈值（需结合模型包围盒归一化）
// 如果你希望设置一个“当代价超过 \(X\) 时停止”的绝对阈值，你必须先在初始化时计算出模型包围盒的对角线长度 \(L\)。
// 做法：将所有顶点的初始坐标全部除以 \(L\)（即将模型缩放到  +-1 xyz 的空间内）。
// 经验数值：在模型被归一化后：
// 高精度保持：当 Cost 达到 1e-5 到 1e-4 时停止，此时肉眼几乎看不出几何形变。0.00001 ~ 0.0001
//           在数学层面上，新点到周围每个原始平面的垂直距离大约在 0.0014 左右 +-1 xyz
//           不想 对顶点 进行 变换的时候 可以 根据对角线 来 放大 cost , 对角线长度“平方”后再乘原本的 cost 
// 中等简化（游戏/常规资产）：当 Cost 达到 1e-3 到 1e-2 时停止，模型在大轮廓上保持完好，但细节（如平滑表面上的密集三角网）已被大量融合。
// 极限低模（LOD 远处剪影）：当 Cost 超过 0.1 时，模型会发生严重的拓扑坍塌和轮廓扭曲。

// 目标三角面数 / 顶点数阈值（最常用）
//    直接设定“简化到原始面数的 30%”或“目标面数降至 5000 面”。只要队列不为空且面数未达标，就一直弹出来坍缩
// 边界法线翻转保护（Normal Flip Constraint）：当准备坍缩一条边时，计算新顶点会导致周围相邻面片的法线夹角改变超过某个阈值 例如大于 60 度。
//    即使这条边的数学 Cost 非常低，也必须拒绝或直接终止，因为这意味着平面发生了肉眼可见的“折叠”或“翻转”。
// 最大豪斯多夫距离（Hausdorff Distance）：限制简化后的网格表面与原始网格表面的最大物理距离。
//     超过设定的物理距离（例如超过 0.5mm则停止
TEST(QEM, test_qem) {
}
