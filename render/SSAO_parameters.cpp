//
// Created by 潘鑫 on 2026/9/13.
//


#include "scalar.h"
#include "SSAO_parameters.h"

struct BilateralPassConfig {
    uint8_t kernelSize       = 11;
    bool bentNormals         = false;
    float standardDeviation  = 1.0f;
    float bilateralThreshold = 0.0625f;
    float scale              = 1.0f;
};


SSAO_parameters get_SSAO_parameters(AmbientOcclusionOptions const &options, Eigen::Matrix4f projection, float width,
                                    float height) {
    // SSAO_parameters
    float sampleCount{};
    float spiralTurns{};
    float standardDeviation{};

    BilateralPassConfig config = {
        .bentNormals        = options.bentNormals,
        .bilateralThreshold = options.bilateralThreshold,
    };


    switch (options.quality) {
        default:
        case QualityLevel::LOW:
            sampleCount = 7.0f;
            spiralTurns       = 3.0f;
            standardDeviation = 8.0;
            break;
        case QualityLevel::MEDIUM:
            sampleCount = 11.0f;
            spiralTurns       = 6.0f;
            standardDeviation = 8.0;
            break;
        case QualityLevel::HIGH:
            sampleCount = 16.0f;
            spiralTurns       = 7.0f;
            standardDeviation = 6.0;
            break;
        case QualityLevel::ULTRA:
            sampleCount = 32.0f;
            spiralTurns       = 14.0f;
            standardDeviation = 4.0;
            break;
    }
    switch (options.lowPassFilter) {
        default:
        case QualityLevel::LOW:
            // no filtering, values don't matter
            config.kernelSize = 1;
            config.standardDeviation = 1.0f;
            config.scale             = 1.0f;
            break;
        case QualityLevel::MEDIUM:
            config.kernelSize = 11;
            config.standardDeviation = standardDeviation * 0.5f;
            config.scale             = 2.0f;
            break;
        case QualityLevel::HIGH:
        case QualityLevel::ULTRA:
            config.kernelSize = 23;
            config.standardDeviation = standardDeviation;
            config.scale             = 1.0f;
            break;
    }
    const float inc = (1.0f / (sampleCount - 0.5f)) * spiralTurns * filament::math::f::TAU;


    const float peak      = 0.1f * options.radius;
    const float intensity = (filament::math::f::TAU * peak) * options.intensity;

    // always square AO result, as it looks much better
    const float power = options.power * 2.0f;


    //  应该只差这两个参数了
    const float projectionScale = std::min(
                                           0.5f * projection(0, 0) * width,
                                           0.5f * projection(1, 1) * height);


    SSAO_parameters temp;
    temp.sampleCount                = {sampleCount, 1.0f / (sampleCount - 0.5f)};
    temp.angleIncCosSin             = {std::cos(inc), std::sin(inc)};
    temp.invRadiusSquared           = 1.0f / (options.radius * options.radius);
    temp.minHorizonAngleSineSquared = std::pow(std::sin(options.minHorizonAngleRad), 2.0f);
    temp.peak2                      = peak * peak;
    temp.projectionScale            = projectionScale;
    temp.projectionScaleRadius      = projectionScale * options.radius;
    temp.bias                       = options.bias;
    temp.power                      = power;
    temp.intensity                  = intensity / sampleCount;
    temp.spiralTurns                = spiralTurns;
    temp.maxLevel                   = 2; // 这里的值不对

    return temp;
}
