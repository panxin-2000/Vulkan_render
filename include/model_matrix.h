//
// Created by 潘鑫 on 2026/2/17.
//

#ifndef HELLO_MAC_MODEL_MATRIX_H
#define HELLO_MAC_MODEL_MATRIX_H
#include <cmath>
#include "base_element/point_3.h"


struct Quaternion {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;
};

struct scale {
    float x = 1.0f;
    float y = 1.0f;
    float z = 1.0f;
};


inline float *model_matrix_4x4(float *result, const Point_3 t, const Quaternion rotate, const scale s) {
    // 输入: 四元数 q (x, y, z, w)
    // 输出: 3x3 矩阵 (列主序数组 m[9])

    float x2 = rotate.x + rotate.x; // 2x
    float y2 = rotate.y + rotate.y; // 2y
    float z2 = rotate.z + rotate.z; // 2z

    float xx = rotate.x * x2;
    float xy = rotate.x * y2;
    float xz = rotate.x * z2;
    float yy = rotate.y * y2;
    float yz = rotate.y * z2;
    float zz = rotate.z * z2;
    float wx = rotate.w * x2;
    float wy = rotate.w * y2;
    float wz = rotate.w * z2;

    // 第一列 (X-basis)
    result[0] = (1.0f - (yy + zz)) * s.x;
    result[1] = (xy + wz) * s.x;
    result[2] = (xz - wy) * s.x;
    result[3] = 0;

    // 第二列 (Y-basis)
    result[4] = (xy - wz) * s.y;
    result[5] = (1.0f - (xx + zz)) * s.y;
    result[6] = (yz + wx) * s.y;
    result[7] = 0;


    // 第三列 (Z-basis)
    result[8]  = (xz + wy) * s.z;
    result[9]  = (yz - wx) * s.z;
    result[10] = (1.0f - (xx + yy)) * s.z;
    result[11] = 0;

    // 第四列
    result[12] = t.x;
    result[13] = t.y;
    result[14] = t.z;
    result[15] = 1;
    return result;
}

/**
 * 相机的x轴 指向屏幕的右边，y轴指向屏幕上方，那么z的正方向是 屏幕向 人的眼睛
 * @param result
 * @param t
 * @param q
 * @return
 */
inline float *view_matrix_4x4(float *result, const Point_3 t, const Quaternion q) {
    // 输入: 四元数 q (x, y, z, w)
    // 输出: 3x3 矩阵 (列主序数组 m[9])

    float x2 = q.x + q.x; // 2x
    float y2 = q.y + q.y; // 2y
    float z2 = q.z + q.z; // 2z

    float xx = q.x * x2;
    float xy = q.x * y2;
    float xz = q.x * z2;
    float yy = q.y * y2;
    float yz = q.y * z2;
    float zz = q.z * z2;
    float wx = q.w * x2;
    float wy = q.w * y2;
    float wz = q.w * z2;

    // 第一列 (X-basis)
    result[0] = (1.0f - (yy + zz));
    result[4] = (xy + wz);
    result[8] = (xz - wy);
    result[3] = 0;

    // 第二列 (Y-basis)
    result[1] = (xy - wz);
    result[5] = (1.0f - (xx + zz));
    result[9] = (yz + wx);
    result[7] = 0;


    // 第三列 (Z-basis)
    result[2]  = (xz + wy);
    result[6]  = (yz - wx);
    result[10] = (1.0f - (xx + yy));
    result[11] = 0;

    // 上面其实直接给出了逆矩阵，

    // 相机的位置一般是放在正半轴，看向原点，
    result[12] = -(result[0] * t.x + result[4] * t.y + result[8] * t.z);
    result[13] = (result[1] * t.x + result[5] * t.y + result[9] * t.z);
    result[14] = -(result[2] * t.x + result[6] * t.y + result[10] * t.z);
    result[15] = 1;
    return result;
}

// 四元数归一化
inline void quat_normalize(Quaternion *q) {
    float mag    = sqrtf(q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w);
    float invMag = 1.0f / mag;
    q->x         *= invMag;
    q->y         *= invMag;
    q->z         *= invMag;
    q->w         *= invMag;
}

inline Quaternion quat_mul(Quaternion a, Quaternion b) {
    Quaternion r;
    r.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
    r.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
    r.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
    r.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
    return r;
}

inline Quaternion quat_from_axis_angle_x(float angle) {
    Quaternion q;
    float halfAngle = angle * 0.5f;
    float s         = sinf(halfAngle);
    q.w             = cosf(halfAngle);
    q.x             = s;
    q.y             = 0;
    q.z             = 0;
    return q;
}

inline Quaternion quat_from_axis_angle_y(float angle) {
    Quaternion q;
    float halfAngle = angle * 0.5f;
    float s         = sinf(halfAngle);
    q.w             = cosf(halfAngle);
    q.x             = 0;
    q.y             = s;
    q.z             = 0;
    return q;
}

// 需要确定要放置在哪里

// Quaternion g_cameraRotation = {0.0f, 0.0f, 0.0f, 1.0f};

// void onMouseMove(float deltaX, float deltaY) {
//     float sensitivity = 0.002f;
//
//     // 1. 创建增量旋转
//     // 左右滑动 (Yaw) 绕相机的上轴 (Y)
//     Quaternion qYaw = quat_from_axis_angle_y(-deltaX * sensitivity);
//
//     // 上下滑动 (Pitch) 绕相机的右轴 (X)
//     Quaternion qPitch = quat_from_axis_angle_x(-deltaY * sensitivity);
//
//     // 2. 核心：将增量应用到当前旋转 (Local Space Multiplication)
//     // 顺序：当前旋转 * 偏航 * 俯仰
//     g_cameraRotation = quat_mul(g_cameraRotation, qYaw);
//     g_cameraRotation = quat_mul(g_cameraRotation, qPitch);
//
//     // 3. 归一化防止累积误差（浮点数精度漂移）
//     quat_normalize(&g_cameraRotation);
// }


struct alignas(16) matrix_4x4 {
    float p[16];
};

inline matrix_4x4 &model_matrix_4x4_reference(float *result, const Point_3 t, const Quaternion q,
                                              const scale s) {
    matrix_4x4 &return_value = *reinterpret_cast<matrix_4x4 *>(model_matrix_4x4(result, t, q, s));
    return return_value;
}


// 简单来说我画的立方体其实是左手坐标系（xy没问题，但是z是有问题的），而不是右手坐标系才有这么多问题？ 确实还是自己做起来收获比较多。
inline matrix_4x4 perspective_matrix_4x4(float *result, float fovy, float aspect, float zNear, float zFar) {
    // 简单的一点都做法就是填值就好了
    float u    = tan(fovy / 2.0f) * zNear;
    float d    = -u;
    float r    = aspect * u;
    float l    = -r;
    result[0]  = zNear / r; // 1.0f / (aspect * (tan(fovy / 2.0f));      // 问题是 zNear 为什么被强制设置为1了
    result[1]  = 0;
    result[2]  = 0;
    result[3]  = 0;
    result[4]  = 0;
    result[5]  = zNear / u; // 为什么算法会不一样  = 1.0f / (tan(fovy / 2.0f))  // 原因是什么？
    result[6]  = 0;
    result[7]  = 0;
    result[8]  = 0;
    result[9]  = 0;
    result[10] = -(zFar + zNear) / (zFar - zNear);
    result[12] = 0;
    result[13] = 0;
    result[14] = -2 * zFar * zNear / (zFar - zNear);
    result[11] = -1;
    result[15] = 0;


    return *reinterpret_cast<matrix_4x4 *>(result);
}

inline matrix_4x4 orthographic_matrix_4x4(float *result, float fovy, float aspect, float zNear, float zFar) {
    // 简单的一点都做法就是填值就好了
    float u    = tan(fovy / 2.0f) * zNear;
    float d    = -u;
    float r    = aspect * u;
    float l    = -r;
    result[0]  = 1 / r; // 问题是 zNear 为什么被强制设置为1了
    result[5]  = 1 / u;
    result[10] = -2.0f / (zFar - zNear);
    result[14] = -1 * (zFar + zNear) / (zFar - zNear);

    return *reinterpret_cast<matrix_4x4 *>(result);
}

inline matrix_4x4 identity_matrix_4x4(matrix_4x4 *result_m) {
    auto result =reinterpret_cast<float *>(result_m);
    result[0]  = 1;    result[4]  = 0;       result[8]  = 0;    result[12] = 0;
    result[1]  = 0;    result[5]  = 1;       result[9]  = 0;    result[13] = 0;
    result[2]  = 0;    result[6]  = 0;       result[10] = 1;    result[14] = 0;
    result[3]  = 0;    result[7]  = 0;       result[11] = 0;    result[15] = 1;
    return *reinterpret_cast<matrix_4x4 *>(result);
}


inline matrix_4x4 UI_matrix_4x4(matrix_4x4 *result_m, const Point_2 zoom,const Point_2 offset) {
    auto result       =reinterpret_cast<float *>(result_m);
    result[0]   = zoom.x;    result[4] = 0;        result[8]     = 0;    result[12] = offset.x;
    result[1]   = 0;         result[5] = zoom.y;   result[9]     = 0;    result[13] = offset.y;
    result[2]   = 0;         result[6] = 0;        result[10]    = 1;    result[14] = 0;
    result[3]   = 0;         result[7] = 0;        result[11]    = 0;    result[15] = 1;
    return *reinterpret_cast<matrix_4x4 *>(result);
}


inline matrix_4x4 UI_projection_4x4(matrix_4x4 *result_m, const float zoom_x, const float zoom_y) {
    auto result       =reinterpret_cast<float *>(result_m);

    result[0]   = 2.0f/zoom_x;    result[4] = 0;                result[8]       = 0;    result[12] =  - 1.0f;
    result[1]   = 0;              result[5] = 2.0f/zoom_y;      result[9]       = 0;    result[13] =  - 1.0f;
    result[2]   = 0;              result[6] = 0;                result[10]      = 1;    result[14] = 0;
    result[3]   = 0;              result[7] = 0;                result[11]      = 0;    result[15] = 1;
    return *reinterpret_cast<matrix_4x4 *>(result);
}



#endif //HELLO_MAC_MODEL_MATRIX_H
