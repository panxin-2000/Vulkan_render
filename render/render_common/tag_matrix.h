//
// Created by 潘鑫 on 2026/9/23.
//

#ifndef HELLO_MAC_TAG_MATRIX_H
#define HELLO_MAC_TAG_MATRIX_H
#include <Eigen/Eigen>
#include "global_singleton.h"

struct InverseBindMatrix {
    Eigen::Matrix4f matrix;
};

struct Inverse_Global_Transform {
    Eigen::Matrix4f matrix;
};

struct JointMatrix {
    Eigen::Matrix4f matrix;
};

struct JointMatrixDirty {
};

struct ModelMatricesDirty {
};

struct Add_new_model {
};


#include <Eigen/Dense>

inline Eigen::Vector3f InterpolateCubicSplineVector(
    float s,
    const Eigen::Vector3f &v_k,        // Current Value
    const Eigen::Vector3f &out_tan_k,  // Current OutTangent
    const Eigen::Vector3f &in_tan_kp1, // Next InTangent
    const Eigen::Vector3f &v_kp1,      // Next Value
    float deltaTime = 1.0f
) {
    float s2 = s * s;
    float s3 = s2 * s;

    // 混合系数
    float h00 = 2 * s3 - 3 * s2 + 1;
    float h10 = s3 - 2 * s2 + s;
    float h01 = -2 * s3 + 3 * s2;
    float h11 = s3 - s2;

    // 关键点：切线必须乘以 deltaTime
    Eigen::Vector3f b_k   = deltaTime * out_tan_k;
    Eigen::Vector3f a_kp1 = deltaTime * in_tan_kp1;

    // 埃尔米特插值
    return h00 * v_k + h10 * b_k + h01 * v_kp1 + h11 * a_kp1;
}

#include <Eigen/Dense>

inline Eigen::Quaternionf InterpolateCubicSplineQuaternion(
    float s,
    const Eigen::Quaternionf &v_k,
    const Eigen::Quaternionf &out_tan_k,
    const Eigen::Quaternionf &in_tan_kp1,
    const Eigen::Quaternionf &v_kp1,
    float deltaTime = 1.0f
) {
    float s2 = s * s;
    float s3 = s2 * s;

    float h00 = 2 * s3 - 3 * s2 + 1;
    float h10 = s3 - 2 * s2 + s;
    float h01 = -2 * s3 + 3 * s2;
    float h11 = s3 - s2;

    // 将四元数视为 4D 向量进行计算
    Eigen::Vector4f q0    = v_k.coeffs();
    Eigen::Vector4f out_t = out_tan_k.coeffs();
    Eigen::Vector4f in_t  = in_tan_kp1.coeffs();
    Eigen::Vector4f q1    = v_kp1.coeffs();

    Eigen::Vector4f b_k   = deltaTime * out_t;
    Eigen::Vector4f a_kp1 = deltaTime * in_t;

    // 混合 4D 向量
    Eigen::Vector4f result_vec = h00 * q0 + h10 * b_k + h01 * q1 + h11 * a_kp1;

    // 还原为四元数并【必须】归一化
    Eigen::Quaternionf result_q(result_vec[3], result_vec[0], result_vec[1], result_vec[2]); // w, x, y, z
    result_q.normalize();

    return result_q;
}


/**
 * 这里的存储的是已经 上传到GPU 上的 material 的索引
 */
class Gpu_material_indices : public std::vector<uint32_t> {
public:
    using std::vector<uint32_t>::vector;
};

/**
 * 这里存储的 gltf 的 assert 中 的 material 的索引, 需要通过上一个转化为 engine 的才能上传到GPU
 */
class Gltf_material_parameters : public std::vector<uint32_t> {
public:
    using std::vector<uint32_t>::vector;
};

class Skin_matrix_vector_index : public std::vector<entt::entity> {
public:
    using std::vector<entt::entity>::vector;
};

class screen_pick_entity : public std::vector<entt::entity> {
public:
    using std::vector<entt::entity>::vector;
};


#endif //HELLO_MAC_TAG_MATRIX_H
