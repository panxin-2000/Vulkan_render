//
// Created by 潘鑫 on 2026/9/13.
//

#ifndef HELLO_MAC_SSAO_PARAMETERS_H
#define HELLO_MAC_SSAO_PARAMETERS_H
#include <Eigen/Eigen>

enum class QualityLevel : uint8_t {
    LOW,
    MEDIUM,
    HIGH,
    ULTRA
};

/**
 * Options for screen space Ambient Occlusion (SSAO) and Screen Space Cone Tracing (SSCT)
 * @see #setAmbientOcclusionOptions
 */
struct AmbientOcclusionOptions {
    enum class AmbientOcclusionType : uint8_t {
        /** use Scalable Ambient Occlusion */
        SAO,
        /** use Ground Truth-Based Ambient Occlusion */
        GTAO,
    };

    /** Type of ambient occlusion algorithm. */
    AmbientOcclusionType aoType = AmbientOcclusionType::SAO;
    /** Ambient Occlusion radius in meters, between 0 and ~10. */
    float radius = 0.3f;
    /** Controls ambient occlusion's contrast. Must be positive. */
    float power = 1.0f;

    /**
     * Self-occlusion bias in meters. Use to avoid self-occlusion.
     * Between 0 and a few mm. No effect when aoType set to GTAO
     */
    float bias = 0.0005f;

    /** How each dimension of the AO buffer is scaled. Must be either 0.5 or 1.0. */
    float resolution = 0.5f;
    /** Strength of the Ambient Occlusion effect. */
    float intensity = 1.0f;
    /** depth distance that constitute an edge for filtering */
    float bilateralThreshold = 0.05f;
    /** affects # of samples used for AO and params for filtering */
    QualityLevel quality = QualityLevel::LOW;
    /** affects AO smoothness. Recommend setting to HIGH when aoType set to GTAO. */
    QualityLevel lowPassFilter = QualityLevel::MEDIUM;
    /** affects AO buffer upsampling quality */
    QualityLevel upsampling = QualityLevel::LOW;
    /** enables or disables screen-space ambient occlusion */
    bool enabled = false;
    /** enables bent normals computation from AO, and specular AO */
    bool bentNormals = false;
    /** min angle in radian to consider. No effect when aoType set to GTAO. */
    float minHorizonAngleRad = 0.0f;
};


struct SSAO_parameters {
    //    mat4 screenFromViewMatrix;
    // Eigen::Vector4f resolution; // { desc.width, desc.height, 1.0f / desc.width, 1.0f / desc.height } 改为现场计算吧
    //    vec2 positionParams;
    Eigen::Vector2f sampleCount; // sampleCount, 1.0f / (sampleCount - 0.5f)
    // const float inc = (1.0f / (sampleCount - 0.5f)) * spiralTurns * f::TAU;
    Eigen::Vector2f angleIncCosSin; // std::cos(inc), std::sin(inc)

    float invRadiusSquared;           // 1.0f / (options.radius * options.radius)
    float minHorizonAngleSineSquared; // std::pow(std::sin(options.minHorizonAngleRad), 2.0f)
    float peak2;                      // 0.0300000012 * 0.0300000012
    float projectionScale;            // projectionScale

    float projectionScaleRadius; // projectionScale * options.radius
    float bias;                  // 0.0005f
    float power;                 // 2
    float intensity;             // ((f::TAU * peak) * options.intensity) / sampleCount

    float spiralTurns; // spiralTurns
    float maxLevel;    // 其实这里可以先空起来,看看慢的情况下能不能得到, 之后
    Eigen::Vector2f reserved;
    //    float ssctShadowDistance;
    //    float ssctConeAngleTangeant;
    //    float ssctContactDistanceMaxInv;
    //    vec3 ssctVsLightDirection;
    //    float ssctIntensity;
    //    vec2 ssctDepthBias;
    //    vec2 ssctRayCount;
    //    uint ssctSampleCount;
};


SSAO_parameters get_SSAO_parameters(AmbientOcclusionOptions const &options, Eigen::Matrix4f projection, float width,
                                    float height);

#endif //HELLO_MAC_SSAO_PARAMETERS_H
