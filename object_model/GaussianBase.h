// Vulkan 3DGS - Copyright (c) 2025 Alejandro Amat (github.com/AlejandroAmat) -
// MIT Licensed

#pragma once
#include <iostream>
#include <vector>
#include <Eigen/Eigen>

class GaussianBase {
public:
    GaussianBase() {
    };

    // GPU upload interface
    [[nodiscard]] const void *GetPositionsData() const { return _xyz.data(); }
    [[nodiscard]] const void *GetScalesData() const { return _scales.data(); }
    [[nodiscard]] const void *GetRotationsData() const { return _rotations.data(); }
    [[nodiscard]] const void *GetOpacitiesData() const { return _opacities.data(); }
    [[nodiscard]] const void *GetSHData() const { return _shCoefficients.data(); }

    size_t GetCount() const { return _numGaussians; }
    int GetSHDegree() const { return _shDegree; }

    int GetSHCoefficientsPerChannel() const {
        return (_shDegree + 1) * (_shDegree + 1);
    }


    [[nodiscard]] std::pair<Eigen::Vector3f, Eigen::Vector3f> get_min_max() const {
        Eigen::Vector4f minBounds(std::numeric_limits<float>::max(),
                                  std::numeric_limits<float>::max(),
                                  std::numeric_limits<float>::max(), 0.0f);
        Eigen::Vector4f maxBounds(-std::numeric_limits<float>::max(),
                                  -std::numeric_limits<float>::max(),
                                  -std::numeric_limits<float>::max(), 0.0f);
        // 2. 遍历计算（确保 _xyz 不为空）
        if (!_xyz.empty()) {
            for (const auto &pt: _xyz) {
                minBounds = minBounds.cwiseMin(pt);
                maxBounds = maxBounds.cwiseMax(pt);
            }
        }
        // 3. 获取前三个维度的 XYZ 结果
        Eigen::Vector3f minXYZ = minBounds.head<3>();
        Eigen::Vector3f maxXYZ = maxBounds.head<3>();
        return std::make_pair(minXYZ, maxXYZ);
    }


    ~GaussianBase() {
    };

    // Data vectors
    std::vector<Eigen::Vector4f> _xyz;
    std::vector<Eigen::Vector3f> _normals; // 好像并没有使用
    std::vector<float> _shCoefficients;
    std::vector<float> _opacities;
    std::vector<Eigen::Vector4f> _scales;
    std::vector<Eigen::Vector4f> _rotations;

    size_t _numGaussians = 0;
    int _maxSHDegree     = 3;
    int _shDegree        = 0;

    enum DATA_TYPE {
        position_xyz = 1,
        normals,
        dc_coeffs,
        SH,
        opacity,
        scales,
        rotations,
    };

    std::vector<DATA_TYPE> data_types;
};
