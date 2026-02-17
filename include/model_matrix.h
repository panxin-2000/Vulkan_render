//
// Created by 潘鑫 on 2026/2/17.
//

#ifndef HELLO_MAC_MODEL_MATRIX_H
#define HELLO_MAC_MODEL_MATRIX_H

struct translation {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct rotation {
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


inline float *model_matrix_4x4(float *result, const translation t, const rotation q, const scale s) {
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


struct model_matrices_4x4 {
    float p[16];
};

inline model_matrices_4x4 &model_matrix_4x4_reference(float *result, const translation t, const rotation q,
                                                      const scale s) {
    model_matrices_4x4 &return_value = *reinterpret_cast<model_matrices_4x4 *>(model_matrix_4x4(result, t, q, s));
    return return_value;
}

#endif //HELLO_MAC_MODEL_MATRIX_H
