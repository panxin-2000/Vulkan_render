//
// Created by 潘鑫 on 2026/3/10.
//

#ifndef HELLO_MAC_PBR_COMPONENT_H
#define HELLO_MAC_PBR_COMPONENT_H
#include "global_singleton.h"
#include "vulkan_image.h"
#include "PBR_base.h"


void set_PBR_base_color(const entt::entity entity, Color baseColorFactor = {1.0f, 1.0f, 1.0f, 1.0f});


void set_PBR_Emissive_color(const entt::entity entity, Color EmissiveFactor = {1.0f, 1.0f, 1.0f, 1.0f});

void set_PBR_metallic_roughness_occlusion(const entt::entity entity,
                                          float metallic  = 1.0f,
                                          float roughness = 1.0f,
                                          float occlusion = 1.0f);

void set_baseColor_Texture_index(const entt::entity entity, const std::optional<Texture_parameter> &texture);

void set_normal_Texture_index(const entt::entity entity, const std::optional<Texture_parameter> &texture);

void set_emissive_Texture_index(const entt::entity entity, const std::optional<Texture_parameter> &texture);

void set_ORM_Texture_index(const entt::entity entity, const std::optional<Texture_parameter> &texture);

//

class Light {
    Eigen::Vector4f position_;
    Eigen::Vector4f rotate_ = {0, 1, 0, 0};
    Color color_;
    float intensity_;
    float range_ = std::numeric_limits<float>::infinity();
    float innerConeAngle_;
    float outerConeAngle_;
    // color_.LightType 应该是 这样的 三个值
    //   -1.0f  0.0f  1.0f
    //                夹角
    //   算了 直接 三个不同的 buffer 来存放就好

private:
    float get_att_distance(float distance) {
        float distanceSq   = distance * distance;
        float rangeSq      = range_ * range_;
        float factor       = distanceSq / rangeSq;
        float smoothFactor = std::clamp(1.0f - factor * factor, 0.0f, 1.0f);
        return (smoothFactor * smoothFactor) / std::max(distanceSq, 0.0001f);
    }

    // cd: 夹角余弦, cosInner: 内角余弦, cosOuter: 外角余弦
    float getAngleAttenuation(float cd, float cosInner, float cosOuter) {
        // 线性映射并限制在 0~1 之间
        float scale  = 1.0f / std::max(cosInner - cosOuter, 0.0001f);
        float offset = -cosOuter * scale;
        // 这两个是可以先算好的 直接将计算好的   scale  和 offset 传递给 shader
        //

        float attenuation = std::clamp(cd * scale + offset, 0.0f, 1.0f);
        auto temp         = std::clamp(color_.LightType, 0.0f, 1.0f);
        return attenuation * attenuation * color_.LightType; // 平滑衰减
    }

public:
    Light() {
    }

    bool operator==(const Light &rhs) const {
        return std::memcmp(this, &rhs, sizeof(Light)) == 0;
    }

    void set_color(const float R, const float G, const float B) {
        color_.R = R;
        color_.G = G;
        color_.B = B;
    }

    void set_position(const float x, const float y, const float z) {
        position_ = {x, y, z, 1.0f};
    }

    void set_position(const Eigen::Vector4f &position) {
        position_ = position;
    }

    void set_rotate(const Eigen::Vector4f &rotate) {
        rotate_ = rotate;
    }

    Eigen::Vector3f get_direction() {
        return rotate_.head<3>();
    }


    /**
     *
     * @param type  Directional = 0.0f   Spot = 1.0f  Point = 2.0f
     */
    void ser_color_type(const float type) {
        color_.LightType = type;
    }

    void set_intensity(const float intensity) {
        intensity_ = intensity;
    }

    void set_range(const float range) {
        range_ = range;
    }

    /**
     *
     * @param innerConeAngle 半角（Half-angle）弧度值
     */
    void set_innerConeAngle(const float innerConeAngle) {
        innerConeAngle_ = innerConeAngle;

        // float innerRad = light.innerConeAngle.value_or(0.0f);
        // // 默认外角是 45 度（PI / 4 弧度）
        // float outerRad = light.outerConeAngle.value_or(0.78539816f);
        //
        // // 1. 在 CPU 端先算好余弦值
        // float cosInner = std::cos(innerRad);
        // float cosOuter = std::cos(outerRad);
        //
        // // 2. 包装成 GPU 友好的线性映射系数 (防除以 0 保护)
        // float lightAngleScale  = 1.0f / std::max(cosInner - cosOuter, 0.0001f);
        // float lightAngleOffset = -cosOuter * lightAngleScale;
        //
        // // 3. 将 lightAngleScale 和 lightAngleOffset 传入 Shader Uniform 中
    }

    void set_outerConeAngle(const float outerConeAngle) {
        outerConeAngle_ = outerConeAngle;
    }
};


#endif //HELLO_MAC_PBR_COMPONENT_H
