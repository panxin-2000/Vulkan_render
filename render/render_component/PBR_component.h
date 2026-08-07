//
// Created by 潘鑫 on 2026/3/10.
//

#ifndef HELLO_MAC_PBR_COMPONENT_H
#define HELLO_MAC_PBR_COMPONENT_H
#include "global_singleton.h"
#include "transform_component.h"
#include "vulkan_image.h"


struct Color {
    float R;
    float G;
    float B;
    float LightType; // 最后一个不用，但是需要占位对齐
};

struct PBR_material_index {
    uint32_t index;
};

class alignas(16) PBR_component {
public:
    // Factor_ 是需要保留的

    Color baseColorFactor_ = {1.0f, 1.0f, 1.0f, 1.0f}; // 基础颜色 和 透明度
    //                         非金属时：代表漫反射颜色。
    //                         金属时：代表反射光的颜色（金属几乎没有漫反射）。
    Color emissiveFactor_ = {0.0f, 0.0f, 0.0f, 1.0f}; // 自发光

    //   渲染方程中的 材料属性输入
    float metallicFactor_ = 0.5f; // 纯白 (1.0) 代表金属，纯黑 (0.0) 代表非金属。
    //                         中间值极少使用，仅用于锈迹或灰尘等过渡效果
    // 把 Metallic 调为 1 时，系统会自动提取 Base Color 的颜色作为反射光颜色
    float roughnessFactor_ = 1.0f; // 粗糙度
    //                         值越高 (1.0/白色)：表面越粗糙，反射光越分散（亚光感）
    //                         值越低 (0.0/黑色)：表面越光滑，反射越清晰（镜面感）
    float occlusion_strength_ = 1.0f; // 强度的公式稍微有点不一样

    float alphaCutoff    = 1.0f;
    float ior            = 1.0f;
    uint32_t doubleSided = false; // 是否开始背面剪裁， 叶子、旗帜、纸张等超薄物体 需要为 true
    uint32_t alphaMode   = 0;     // 有三个值
    //                              OPAQUE (不透明 - 默认)
    //                              MASK  基于 alphaCutoff 阈值进行“全有或全无”的硬切
    //                              BLEND (混合/半透明)
    // 需要一个为全为一的贴图，也就是纯白的贴图
    uint32_t baseColorTexture = 0; // 基础颜色 贴图
    uint32_t normalTexture    = 1; //

    uint32_t emissiveTexture = 0; // 自发光 贴图
    uint32_t ORM_Texture     = 0; // Occlusion, Roughness, Metallic
    // 视差贴图 位移贴图

    // 清漆贴图 (Clearcoat Texture)：模拟车漆表面的透明涂层。
    // 透射贴图 (Transmission Texture)：用于玻璃、水等半透明材质。
    // 厚度贴图 (Thickness Texture)：配合 KHR_materials_volume 扩展，定义体积材质的厚度。
    // 光泽贴图 (Sheen Texture)：模拟丝绒、织物的边缘反光。

    // uint8_t AlphaTexture;     // 没有专门的一张，baseColorTexture的第四通道

    // ORM 合用一张贴图的三个不同通道 必须是 Linear (线性)
    // R          G          B
    // Occlusion, Roughness, Metallic
    // Occlusion 的值是线性采样的，不需要 sRGB 转换
    // 值为 0.0 表示完全遮蔽（无间接光），值为 1.0 表示完全无遮蔽（接收全部间接光）
};

struct PBR_Texture_ptr {
    Texture_parameter baseColorTexture;
    Texture_parameter normalTexture;
    Texture_parameter emissiveTexture;
    Texture_parameter ORM_Texture;
};

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
