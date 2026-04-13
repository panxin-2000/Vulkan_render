//
// Created by 潘鑫 on 2026/4/13.
//

#include "spherical_harmonics.h"


#include "sh/spherical_harmonics.h"

// 假设 order = 3 (产生 9 个系数)
sh::SHVectorRGB projectCubeMapToSH(const MyCubeMap &cubemap, int order) {
    sh::SHVectorRGB sh_coeffs(order);
    float totalWeight = 0.0f;

    // 1. 遍历 CubeMap 的 6 个面
    for (int face = 0; face < 6; ++face) {
        for (int y = 0; y < cubemap.height; ++y) {
            for (int x = 0; x < cubemap.width; ++x) {
                // 2. 获取该像素对应的球面方向向量 dir (x, y, z)
                Eigen::Vector3d dir = cubemap.getDirection(face, x, y);

                // 3. 计算该像素的立体角权重 (Solid Angle)
                // 注意：CubeMap 像素在球面上分布不均，必须加权，否则系数会失真
                double weight = cubemap.getSolidAngleWeight(face, x, y);

                // 4. 获取颜色值 (线性空间)
                Eigen::Vector3d color = cubemap.getPixelColor(face, x, y);

                // 5. 调用 Google 库的 AddLight
                // 它内部会自动计算基函数 Ylm(dir) 并累加：coeffs += color * Ylm(dir) * weight
                sh_coeffs.AddLight(dir, color, weight);

                totalWeight += weight;
            }
        }
    }

    // 6. 归一化 (整个球面的总立体角是 4 * PI)
    sh_coeffs *= (4.0 * M_PI / totalWeight);

    // 7. 重要：进行漫反射卷积 (Convolution)
    // 执行完这一步，系数才可以直接在 Shader 里作为 Irradiance 使用
    sh::ConvolveWithCosineKernel(&sh_coeffs);

    return sh_coeffs;
}
