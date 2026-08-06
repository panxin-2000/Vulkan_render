//
// Created by 潘鑫 on 2026/5/28.
//

#include <gtest/gtest.h>
#include "spherical_SH.h"
#include <Eigen/Dense>
#include <vector>
#include <cmath>


struct Picture_parameters {
    int width;
    int height;
    int channels;
    uint8_t *image_data;
};

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// 快捷别名
using Eigen::Vector3f;
using Eigen::Array3f;

// 1. 采集/投影天空盒的函数
std::vector<Array3f> ProjectSkyboxToGoogleSH(const float *cubemapData, int faceSize, int order) {
    // 根据阶数（Order）计算需要的系数个数，例如 3阶（L2）需要 9 个系数
    int numCoefficients = order * order;
    std::vector<Array3f> shCoefficients(numCoefficients, Array3f::Zero());

    float weightSum = 0.0f;

    // 遍历立方体贴图的 6 个面
    for (int face = 0; face < 6; ++face) {
        for (int v = 0; v < faceSize; ++v) {
            for (int u = 0; u < faceSize; ++u) {
                // 将 U, V 转换到 [-1, 1] 空间并映射为 3D 方向
                float invSize = 1.0f / faceSize;
                float x_coord = (u + 0.5f) * invSize * 2.0f - 1.0f;
                float y_coord = (v + 0.5f) * invSize * 2.0f - 1.0f;

                Eigen::Vector3d dir;
                switch (face) {
                    case 0: dir = Eigen::Vector3d(1.0f, -y_coord, -x_coord);
                        break; // +X
                    case 1: dir = Eigen::Vector3d(-1.0f, -y_coord, x_coord);
                        break; // -X
                    case 2: dir = Eigen::Vector3d(x_coord, 1.0f, y_coord);
                        break; // +Y
                    case 3: dir = Eigen::Vector3d(x_coord, -1.0f, -y_coord);
                        break; // -Y
                    case 4: dir = Eigen::Vector3d(x_coord, -y_coord, 1.0f);
                        break; // +Z
                    case 5: dir = Eigen::Vector3d(-x_coord, -y_coord, -1.0f);
                        break; // -Z
                }

                float length = dir.norm();
                dir.normalize(); // 单位化方向向量，传入给 Google 库

                // 计算当前像素对应的球面立体角权重，修正立方体畸变
                float deltaOmega = 4.0f / ((x_coord * x_coord + y_coord * y_coord + 1.0f) * length * faceSize *
                                           faceSize);

                // 假设这是你从天空盒获取线性和未压缩颜色的函数
                Array3f rgbColor{1.0f, 1.0f, 1.0f};

                // -------------- 核心核心：调用 Google 库计算球谐基 --------------
                // sh::EvalSH 函数会自动计算给定方向在指定阶数下的所有基函数值
                for (int l = 0; l < order; ++l) {
                    for (int m = -l; m <= l; ++m) {
                        // 计算一维数组的索引位置
                        int index = l * (l + 1) + m;

                        // Google 库返回对应的实数球谐基函数值
                        double shBasis = sh::EvalSH(l, m, dir);

                        // 积分累加：颜色 * 基函数值 * 立体角权重
                        shCoefficients[index] += rgbColor * shBasis * deltaOmega;
                    }
                }
                weightSum += deltaOmega;
            }
        }
    }

    // 归一化（整个球面的理论总权重应当为 4 * PI）
    float normFactor = (4.0f * M_PI) / weightSum;
    for (auto &coef: shCoefficients) {
        coef *= normFactor;
    }

    return shCoefficients;
}

namespace sh {
    class DefaultImage : public Image {
    public:
        DefaultImage(int width, int height) : width_(width), height_(height) {
            pixels_.reset(new Eigen::Array3f[width * height]);
        }

        int width() const override { return width_; };

        int height() const override { return height_; };

        Eigen::Array3f GetPixel(int x, int y) const override {
            int index = x + y * width_;
            return pixels_[index];
        };

        void SetPixel(int x, int y, const Eigen::Array3f &v) override {
            int index      = x + y * width_;
            pixels_[index] = v;
        }

    private:
        const int width_;
        const int height_;

        std::unique_ptr<Eigen::Array3f[]> pixels_;
    };
}

TEST(SH, matrix) {
    GTEST_SKIP();
    Picture_parameters picture_parameters{};
    picture_parameters.image_data = stbi_load("/Users/panxin/Downloads/pano.jpg",
                                              &picture_parameters.width,
                                              &picture_parameters.height,
                                              &picture_parameters.channels, STBI_rgb);


    sh::DefaultImage image(picture_parameters.width, picture_parameters.height);
    for (uint32_t x = 0; x < picture_parameters.width; ++x) {
        for (uint32_t y = 0; y < picture_parameters.height; ++y) {
            Eigen::Array3f temp{
                picture_parameters.image_data[
                    x + y * picture_parameters.width + 0] / 255.0f,
                (picture_parameters.image_data[
                     x + y * picture_parameters.width + 1] / 255.0f),
                (picture_parameters.image_data[
                     x + y * picture_parameters.width + 2] / 255.0f),
            };
            image.SetPixel(x, y, temp);
        }
    }

    auto result = sh::ProjectEnvironment(2, image);
    for (auto result1: *result) {
        std::cout << result1 << std::endl;
    }
    uint a = 0;

    std::vector<Eigen::Array4f> shCoefficients;
    shCoefficients.emplace_back(1.73, 1.73, 1.73, 0.0f);
    shCoefficients.emplace_back(-0.05, -0.05, -0.05, 0.0f);
    shCoefficients.emplace_back(-0.16, -0.16, -0.16, 0.0f);
    shCoefficients.emplace_back(0.01, 0.01, 0.01, 0.0f);
    shCoefficients.emplace_back(-0.0, -0.0, -0.0, 0.0f);
    shCoefficients.emplace_back(0.02, 0.02, 0.02, 0.0f);
    shCoefficients.emplace_back(0.03, 0.03, 0.03, 0.0f);
    shCoefficients.emplace_back(-0.01, -0.01, -0.01, 0.0f);
    shCoefficients.emplace_back(0.01, 0.01, 0.01, 0.0f);
}


// 假设 Google 库返回的 16 个系数存放在 resultSH 中
void PreMultiplySH(std::vector<Eigen::Array4f> &resultSH) {
    // 对应 l=0
    resultSH[0] *= 0.282095f;

    // 对应 l=1
    resultSH[1] *= -0.488603f;
    resultSH[2] *= 0.488603f;
    resultSH[3] *= -0.488603f;

    // 对应 l=2
    resultSH[4] *= 1.092548f;
    resultSH[5] *= -1.092548f;
    resultSH[6] *= 0.315392f;
    resultSH[7] *= -1.092548f;
    resultSH[8] *= 0.546274f;

    // 对应 l=3
    // resultSH[9]  *= -0.590044f;
    // resultSH[10] *= 2.890611f;
    // resultSH[11] *= -0.457046f;
    // resultSH[12] *= 0.373176f;
    // resultSH[13] *= -0.457046f;
    // resultSH[14] *= 1.445306f;
    // resultSH[15] *= -0.590044f;
}
