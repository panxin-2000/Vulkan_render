// Vulkan 3DGS - Copyright (c) 2025 Alejandro Amat (github.com/AlejandroAmat) -
// MIT Licensed

#include "PLYLoader.h"
#include <Eigen/Eigen>

std::unique_ptr<GaussianBase> PLYLoader::LoadPLY(const std::string &path,
                                                 int &sh_degree) {
    auto data = std::make_unique<GaussianBase>();

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open PLY file: " << path << std::endl;
        return nullptr;
    }

    // Parse PLY header
    if (!ParseHeader(file, *data)) {
        std::cerr << "Error: Invalid PLY header" << std::endl;
        return nullptr;
    }

    sh_degree = data->_shDegree;
    // Read vertex data
    if (!ReadVertexData(file, *data)) {
        std::cerr << "Error: Failed to read vertex data" << std::endl;
        return nullptr;
    }

    std::cout << "Loaded " << data->_numGaussians << " Gaussians from " << path
            << std::endl;
    return data;
}

bool PLYLoader::ParseHeader(std::ifstream &file, GaussianBase &data) {
    std::string line;
    std::vector<std::string> properties;
    // 这里是打开的 .ply 文件
    // Read header
    while (std::getline(file, line)) {
        if (line.find("element vertex") != std::string::npos) {
            std::istringstream iss(line);
            std::string element, vertex;
            size_t count;
            iss >> element >> vertex >> count;
            data._numGaussians = count;
        } else if (line.find("property") != std::string::npos) {
            std::istringstream iss(line);
            std::string property, data_type, data_name;
            iss >> property >> data_type >> data_name;
            properties.push_back(data_name);
        } else if (line == "end_header") {
            break;
        }
    }
    uint32_t numberProperties = properties.size();
    switch (numberProperties) {
        case 17:
            data._shDegree = 0;
            break;
        case 26:
            data._shDegree = 1;
            break;
        case 41:
            data._shDegree = 2;
            break;
        case 62:
            data._shDegree = 3;
            break;
    }
    uint32_t last_type = 0;
    for (int i = 0; i < properties.size(); ++i) {
        if (properties.at(i + 0) == "x" &&
            properties.at(i + 1) == "y" &&
            properties.at(i + 2) == "z") {
            data.data_types.push_back(GaussianBase::position_xyz);
        } else if (properties.at(i + 0) == "nx" &&
                   properties.at(i + 1) == "ny" &&
                   properties.at(i + 2) == "nz") {
            data.data_types.push_back(GaussianBase::normals);
        } else if (properties.at(i + 0) == "f_dc_0" &&
                   properties.at(i + 1) == "f_dc_1" &&
                   properties.at(i + 2) == "f_dc_2") {
            data.data_types.push_back(GaussianBase::dc_coeffs);
        } else if (properties.at(i + 0) == "scale_0" &&
                   properties.at(i + 1) == "scale_1" &&
                   properties.at(i + 2) == "scale_2") {
            data.data_types.push_back(GaussianBase::scales);
        } else if (properties.at(i + 0) == "opacity") {
            data.data_types.push_back(GaussianBase::opacity);
        } else if (properties.at(i + 0) == "rot_0" &&
                   properties.at(i + 1) == "rot_1" &&
                   properties.at(i + 2) == "rot_2" &&
                   properties.at(i + 3) == "rot_3") {
            data.data_types.push_back(GaussianBase::rotations);
        } else if (properties.at(i + 0) == "f_rest_0" &&
                   properties.at(i + 1) == "f_rest_1" &&
                   properties.at(i + 2) == "f_rest_2" &&
                   properties.at(i + 3) == "f_rest_3" &&
                   properties.at(i + 4) == "f_rest_4" &&
                   properties.at(i + 5) == "f_rest_5" &&
                   properties.at(i + 6) == "f_rest_6" &&
                   properties.at(i + 7) == "f_rest_7" &&
                   properties.at(i + 8) == "f_rest_8" &&
                   properties.at(i + 9) == "f_rest_9" &&
                   properties.at(i + 10) == "f_rest_10" &&
                   properties.at(i + 11) == "f_rest_11" &&
                   properties.at(i + 12) == "f_rest_12" &&
                   properties.at(i + 13) == "f_rest_13" &&
                   properties.at(i + 14) == "f_rest_14" &&
                   properties.at(i + 15) == "f_rest_15" &&
                   properties.at(i + 16) == "f_rest_16" &&
                   properties.at(i + 17) == "f_rest_17" &&
                   properties.at(i + 18) == "f_rest_18" &&
                   properties.at(i + 19) == "f_rest_19" &&
                   properties.at(i + 20) == "f_rest_20" &&
                   properties.at(i + 21) == "f_rest_21" &&
                   properties.at(i + 22) == "f_rest_22" &&
                   properties.at(i + 23) == "f_rest_23" &&
                   properties.at(i + 24) == "f_rest_24" &&
                   properties.at(i + 25) == "f_rest_25" &&
                   properties.at(i + 26) == "f_rest_26" &&
                   properties.at(i + 27) == "f_rest_27" &&
                   properties.at(i + 28) == "f_rest_28" &&
                   properties.at(i + 29) == "f_rest_29" &&
                   properties.at(i + 30) == "f_rest_30" &&
                   properties.at(i + 31) == "f_rest_31" &&
                   properties.at(i + 32) == "f_rest_32" &&
                   properties.at(i + 33) == "f_rest_33" &&
                   properties.at(i + 34) == "f_rest_34" &&
                   properties.at(i + 35) == "f_rest_35" &&
                   properties.at(i + 36) == "f_rest_36" &&
                   properties.at(i + 37) == "f_rest_37" &&
                   properties.at(i + 38) == "f_rest_38" &&
                   properties.at(i + 39) == "f_rest_39" &&
                   properties.at(i + 40) == "f_rest_40" &&
                   properties.at(i + 41) == "f_rest_41" &&
                   properties.at(i + 42) == "f_rest_42" &&
                   properties.at(i + 43) == "f_rest_43" &&
                   properties.at(i + 44) == "f_rest_44") {
            data.data_types.push_back(GaussianBase::SH);
        }
    }

    return data._numGaussians > 0;
}


inline void Read_ply_position_xyz(std::ifstream &file, GaussianBase &data, size_t i) {
    file.read(reinterpret_cast<char *>(&data._xyz[i]), sizeof(Eigen::Vector3f));
    data._xyz[i].w() = 1.0f;
}

inline void Read_ply_normal(std::ifstream &file, GaussianBase &data, size_t i) {
    file.read(reinterpret_cast<char *>(&data._normals[i]), sizeof(Eigen::Vector3f));
}

inline void Read_ply_dc_coeffs(std::ifstream &file, GaussianBase &data, size_t i, const int total_sh_coeffs) {
    Eigen::Vector3f dc_coeffs;
    file.read(reinterpret_cast<char *>(&dc_coeffs), sizeof(Eigen::Vector3f));
    // Store DC coefficients
    size_t sh_offset = i * total_sh_coeffs;

    data._shCoefficients[sh_offset + 0] = dc_coeffs.x(); // f_dc_0
    data._shCoefficients[sh_offset + 1] = dc_coeffs.y(); // f_dc_1
    data._shCoefficients[sh_offset + 2] = dc_coeffs.z(); // f_dc_2
}

inline void Read_ply_SH(std::ifstream &file, GaussianBase &data, size_t i, const int total_sh_coeffs,
                        const int rest_coeffs) {
    std::vector<float> rest_coeffs_vector(rest_coeffs);
    file.read(reinterpret_cast<char *>(rest_coeffs_vector.data()),
              rest_coeffs * sizeof(float));
    size_t sh_offset = i * total_sh_coeffs;

    for (int j = 0; j < rest_coeffs / 3; ++j) {
        int out_idx                                   = (j + 1) * 3; // +1 because DC is at index 0
        data._shCoefficients[sh_offset + out_idx + 0] =
                rest_coeffs_vector[j]; // R
        data._shCoefficients[sh_offset + out_idx + 1] =
                rest_coeffs_vector[j + (rest_coeffs / 3)]; // G
        data._shCoefficients[sh_offset + out_idx + 2] =
                rest_coeffs_vector[j + 2 * (rest_coeffs / 3)]; // B
    }
}


inline void Read_ply_opacity(std::ifstream &file, GaussianBase &data, size_t i) {
    float rawOpacity;
    file.read(reinterpret_cast<char *>(&rawOpacity), sizeof(float));
    data._opacities[i] = 1.0f / (1.0f + std::exp(-rawOpacity)); // Sigmoid
}

inline void Read_ply_scale(std::ifstream &file, GaussianBase &data, size_t i) {
    Eigen::Vector3f rawScales;
    file.read(reinterpret_cast<char *>(&rawScales), sizeof(Eigen::Vector3f));
    data._scales[i] = Eigen::Vector4f::Zero();
    data._scales[i] = Eigen::Vector4f(rawScales.array().exp()[0],
                                      rawScales.array().exp()[1],
                                      rawScales.array().exp()[2],
                                      0.0f);
}

inline void Read_ply_rot(std::ifstream &file, GaussianBase &data, size_t i) {
    Eigen::Vector4f rawRotation;
    file.read(reinterpret_cast<char *>(&rawRotation), sizeof(Eigen::Vector4f));
    data._rotations[i] = rawRotation.normalized();
}


bool PLYLoader::ReadVertexData(std::ifstream &file, GaussianBase &data) {
    const size_t num_gaussians      = data._numGaussians;
    const int shDegree              = data._shDegree;
    const int sh_coeffs_per_channel = (shDegree + 1) * (shDegree + 1);
    const int total_sh_coeffs       = 3 * sh_coeffs_per_channel;       // 3 channels (RGB)
    const int rest_coeffs           = 3 * (sh_coeffs_per_channel - 1); // Exclude DC

    // 这里并没有处理不同的顺序 好吃是分为了SOA
    // Resize vectors
    data._xyz.resize(num_gaussians);
    data._normals.resize(num_gaussians);
    data._shCoefficients.resize(num_gaussians * total_sh_coeffs);
    data._opacities.resize(num_gaussians);
    data._scales.resize(num_gaussians);
    data._rotations.resize(num_gaussians);

    for (size_t i = 0; i < num_gaussians; ++i) {
        for (const auto data_type: data.data_types) {
            switch (data_type) {
                case GaussianBase::position_xyz: {
                    Read_ply_position_xyz(file, data, i);
                    break;
                }
                case GaussianBase::normals: {
                    Read_ply_normal(file, data, i);
                    break;
                }
                case GaussianBase::dc_coeffs: {
                    Read_ply_dc_coeffs(file, data, i, total_sh_coeffs);
                    break;
                }
                case GaussianBase::SH: {
                    Read_ply_SH(file, data, i, total_sh_coeffs, rest_coeffs);
                    break;
                }
                case GaussianBase::opacity: {
                    Read_ply_opacity(file, data, i);
                    break;
                }
                case GaussianBase::scales: {
                    Read_ply_scale(file, data, i);
                    break;
                }
                case GaussianBase::rotations: {
                    Read_ply_rot(file, data, i);
                    break;
                }
                default: {
                };
            }
        }
    }

    return file.good() || file.eof();
}


// ply
// format binary_little_endian 1.0
// element vertex 1244819
// property float x
// property float y
// property float z
// property float nx
// property float ny
// property float nz
// property float f_dc_0
// property float f_dc_1
// property float f_dc_2
// property float f_rest_0
// property float f_rest_1
// property float f_rest_2
// property float f_rest_3
// property float f_rest_4
// property float f_rest_5
// property float f_rest_6
// property float f_rest_7
// property float f_rest_8
// property float f_rest_9
// property float f_rest_10
// property float f_rest_11
// property float f_rest_12
// property float f_rest_13
// property float f_rest_14
// property float f_rest_15
// property float f_rest_16
// property float f_rest_17
// property float f_rest_18
// property float f_rest_19
// property float f_rest_20
// property float f_rest_21
// property float f_rest_22
// property float f_rest_23
// property float f_rest_24
// property float f_rest_25
// property float f_rest_26
// property float f_rest_27
// property float f_rest_28
// property float f_rest_29
// property float f_rest_30
// property float f_rest_31
// property float f_rest_32
// property float f_rest_33
// property float f_rest_34
// property float f_rest_35
// property float f_rest_36
// property float f_rest_37
// property float f_rest_38
// property float f_rest_39
// property float f_rest_40
// property float f_rest_41
// property float f_rest_42
// property float f_rest_43
// property float f_rest_44
// property float opacity
// property float scale_0
// property float scale_1
// property float scale_2
// property float rot_0
// property float rot_1
// property float rot_2
// property float rot_3
// end_header


// 基本的使用方法如下     _gaussianData = PLYLoader::LoadPLY(_pointCloudFile, _degree);
