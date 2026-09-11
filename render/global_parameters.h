//
// Created by 潘鑫 on 2026/8/24.
//

#ifndef HELLO_MAC_GLOBAL_PARAMETERS_H
#define HELLO_MAC_GLOBAL_PARAMETERS_H
#include <Eigen/Eigen>
#include "frustum.h"
#include "PBR_component.h"


#include <algorithm>
#include <cmath>

struct TechnicalDoFParams {
    float focusDistance; // 喂给 Shader 的对焦距离（米）
    float focusRange;    // 喂给 Shader 的清晰夹层范围（米）
    float maxBlurRadius; // 喂给 Shader 的最大像素模糊半径（像素）
    float pad0 = 0;
};


/**
 * @brief 计算物理景深参数
 * @param focusDistanceMeter  当前精确的对焦距离（通过 Ray_cast 或角色位置获取，单位：米）
 * @param renderWidthPixels   当前屏幕渲染的像素宽度（如 1920, 2560, 3840 等）
 * @param projectionMatrix    通过它来获得 50mm 焦距 = 0.050 米
 * @param apertureFNum        f/1.8 光圈
 * @param sensorWidth         36mm 全画幅传感器宽度 = 0.036 米
 * @param cocThreshold        全画幅业界标准容许弥散圆(CoC)直径 = 0.03mm = 0.000030 米
 * @return TechnicalDoFParams 填充好的 Shader 结构体
 */
inline TechnicalDoFParams CalculateDoFParams(const float focusDistanceMeter,
                                             const float renderWidthPixels,
                                             const Eigen::Matrix4f &projectionMatrix,
                                             const float apertureFNum = 1.8f,
                                             const float sensorWidth  = 0.036f,
                                             const float cocThreshold = 0.000030f) {
    TechnicalDoFParams params;

    // P[0][0] 就是矩阵第一行第一列的元素
    float P00 = projectionMatrix(0, 0);

    // 逆推公式：焦距 = P00 * (传感器宽度 / 2)
    float focalLength = P00 * (sensorWidth * 0.5f);


    // 参数 A: 对焦距离直接赋值
    params.focusDistance = focusDistanceMeter;

    // 防止边界情况：如果对焦距离比焦距还近，物理上无法对焦，做保护处理
    if (focusDistanceMeter <= focalLength) {
        params.focusRange    = 0.0f;
        params.maxBlurRadius = 0.0f;
        return params;
    }

    // 2. 计算景深范围 (Focus Range)
    // 物理公式推导中需要用到的中间变量
    float fSquared       = focalLength * focalLength;
    float hyperfocalTerm = apertureFNum * cocThreshold * (focusDistanceMeter - focalLength);

    // 计算前景深极限 (Near Depth of Field Limit)
    float nearLimit = (focusDistanceMeter * fSquared) / (fSquared + hyperfocalTerm);

    // 计算后景深极限 (Far Depth of Field Limit)
    float farLimit;
    if (fSquared <= hyperfocalTerm) {
        // 如果分母小于等于0，说明后景深已经达到了无穷远处 (Infinity)
        farLimit = 10000.0f; // 用一个极大的数代表无穷远
    } else {
        farLimit = (focusDistanceMeter * fSquared) / (fSquared - hyperfocalTerm);
    }

    // 参数 B: 清晰夹层范围 = 后极限 - 前极限
    params.focusRange = std::max(0.0f, farLimit - nearLimit);


    // 3. 计算最大模糊像素半径 (Max Blur Radius)
    // 当背景处于无穷远处时，物理底片上产生的最大弥散圆直径 (C_infinity)
    float maxPhysicalCoCDiameter = fSquared / (apertureFNum * (focusDistanceMeter - focalLength));

    // 将物理尺寸（米）转换为屏幕像素尺寸：
    // 物理直径 / 传感器宽度 = 像素直径 / 屏幕像素宽度
    float maxBlurDiameterPixels = (maxPhysicalCoCDiameter / sensorWidth) * renderWidthPixels;

    // 参数 C: 转换为 Shader 采样需要的像素半径 (Radius = Diameter / 2)
    params.maxBlurRadius = std::max(0.0f, maxBlurDiameterPixels * 0.5f);

    return params;
}


class Global_parameters {
public:
    Eigen::Matrix4f view_matrix;
    Eigen::Matrix4f projection_matrix;
    Eigen::Matrix4f inv_view_matrix;
    Eigen::Matrix4f inv_projection_matrix;
    Eigen::Matrix4f invVP;
    Eigen::Matrix4f light_viewProjMatrix[4];
    FrustumPlanes frustum_planes;
    std::array<FrustumPlanes, 4> light_frustum_planes;
    Eigen::Vector4f world_camera_pos;
    Light light;
    Eigen::Vector4f screen_size;
    Eigen::Vector4f mouse_position = Eigen::Vector4f::Zero();
    float split_depth[4];
    float film_grain_intensity       = 0.2f;
    float camera_vignette_smoothness = 0.2f;
    float camera_vignette_intensity  = 0.4f;
    uint32_t render_timeline;
    // 镜头相关参数
    float aperture          = 1.8;             // 光圈大小 (控制虚化强度)
    float focalLength       = 50.0f / 1000.0f; // 镜头焦距 (控制视野和虚化程度)
    float sensorWidthMeters = 0.035;           // 全画幅 35mm  输入值 0.035
    float maxBlurPixels     = 24;

    // 距离雾相关参数
    float fogStart   = 50;
    float fogEnd     = 200;
    float fogDensity = 0.015;
    uint fogType     = 1; // fogType == 0  线性雾   fogType == 1  指数雾 fogType == 2  指数平方雾

    float fxaaQualitySubpix           = 0.75;
    float fxaaQualityEdgeThreshold    = 0.166;
    float fxaaQualityEdgeThresholdMin = 0.0833;
    float fxaa_pad                    = 0;

    Eigen::Vector4f fogColor = Eigen::Vector4f(0.7, 0.7, 0.7, 1.0);
    std::array<Eigen::Array4f, 9> shCoefficients;
    Eigen::Vector4f last_sun_camera_pos;


    float &get_fxaaQualitySubpix() {
        return fxaaQualitySubpix;
    }

    float &get_fxaaQualityEdgeThreshold() {
        return fxaaQualityEdgeThreshold;
    }

    float &get_fxaaQualityEdgeThresholdMin() {
        return fxaaQualityEdgeThresholdMin;
    }

    // PhysicalCameraParams CameraParams; //
    float &get_aperture() {
        return aperture;
    }

    float &get_sensor_width() {
        return sensorWidthMeters;
    }

    float &get_focal_length() {
        return focalLength;
    }

    float &get_maxBlurPixels() {
        return maxBlurPixels;
    }


    float &get_fog_start() {
        return fogStart;
    }

    float &get_fog_end() {
        return fogEnd;
    }

    float &get_fog_density() {
        return fogDensity;
    }

    uint &get_fog_type() {
        return fogType;
    }

    Eigen::Vector4f &get_fog_color() {
        return fogColor;
    }

    float &get_camera_vignette_smoothness() {
        return camera_vignette_smoothness;
    }

    float &get_camera_vignette_intensity() {
        return camera_vignette_intensity;
    }

    float &get_film_grain_intensity() {
        return film_grain_intensity;
    }

    bool update_DoF_Params() {
        // 下面的距离 需要通过鼠标来获得了
        // DoFParams = CalculateDoFParams(20, screen_size.x(), projection_matrix);
        return true;
    }

    bool set_mouse_position(const float x, const float y) {
        mouse_position.x() = x;
        mouse_position.y() = y;
        return true;
    }

    bool set_render_timeline(const uint32_t &timeline);

    bool set_projection_matrix(const Eigen::Matrix4f &matrix);

    bool set_inv_projection_matrix(const Eigen::Matrix4f &matrix);

    bool set_view_matrix(const Eigen::Matrix4f &matrix);

    bool set_inv_view_matrix(const Eigen::Matrix4f &matrix);

    bool set_invVP(const Eigen::Matrix4f &matrix);

    bool set_world_camera_pos(const Eigen::Vector3f &v3);

    bool set_sun_light(const Eigen::Vector3f &v3);

    bool set_screen_size(const Eigen::Vector2f &screen_size_t);

    bool update_directional_light();;
};


#endif //HELLO_MAC_GLOBAL_PARAMETERS_H
