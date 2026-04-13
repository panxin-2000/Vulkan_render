//
// Created by 潘鑫 on 2026/3/10.
//

#ifndef HELLO_MAC_PBR_COMPONENT_H
#define HELLO_MAC_PBR_COMPONENT_H
#include "global_singleton.h"


struct Color {
    float R;
    float G;
    float B;
    float A; // 最后一个不用，但是需要占位对齐
};


class alignas(16) PBR_component {
public:
    // Factor_ 是需要保留的

    Color baseColorFactor_ = {1.0f, 1.0f, 1.0f, 1.0f}; // 基础颜色 和 透明度
    //                         非金属时：代表漫反射颜色。
    //                         金属时：代表反射光的颜色（金属几乎没有漫反射）。
    Color emissiveFactor_ = {1.0f, 1.0f, 1.0f, 1.0f}; // 自发光

    //   渲染方程中的 材料属性输入
    float metallicFactor_ = 1.0f; // 纯白 (1.0) 代表金属，纯黑 (0.0) 代表非金属。
    //                         中间值极少使用，仅用于锈迹或灰尘等过渡效果
    // 把 Metallic 调为 1 时，系统会自动提取 Base Color 的颜色作为反射光颜色
    float roughnessFactor_ = 1.0f; // 粗糙度
    //                         值越高 (1.0/白色)：表面越粗糙，反射光越分散（亚光感）
    //                         值越低 (0.0/黑色)：表面越光滑，反射越清晰（镜面感）
    float occlusion_strength_ = 1.0f; // 强度的公式稍微有点不一样

    float alphaCutoff    = 1.0f;
    uint32_t doubleSided = false; // 是否开始背面剪裁， 叶子、旗帜、纸张等超薄物体 需要为 true
    uint32_t alphaMode   = 1.0f;  // 有三个值
    //                              OPAQUE (不透明 - 默认)
    //                              MASK  基于 alphaCutoff 阈值进行“全有或全无”的硬切
    //                              BLEND (混合/半透明)
    // 需要一个为全为一的贴图，也就是纯白的贴图
    uint32_t baseColorTexture = 0; // 基础颜色 贴图
    uint32_t normalTexture    = 0; //
    uint32_t emissiveTexture  = 0; // 自发光 贴图
    uint32_t ORM_Texture      = 0; // Occlusion, Roughness, Metallic
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


void set_PBR_base_color(const entt::entity entity, Color baseColorFactor = {1.0f, 1.0f, 1.0f, 1.0f});


void set_PBR_Emissive_color(const entt::entity entity, Color EmissiveFactor = {1.0f, 1.0f, 1.0f, 1.0f});

void set_PBR_metallic_roughness_occlusion(const entt::entity entity,
                                          float metallic  = 1.0f,
                                          float roughness = 1.0f,
                                          float occlusion = 1.0f);

void set_baseColor_Texture_index(const entt::entity entity, uint32_t index = 0);

void set_normal_Texture_index(const entt::entity entity, uint32_t index = 0);

void set_emissive_Texture_index(const entt::entity entity, uint32_t index = 0);

void set_ORM_Texture_index(const entt::entity entity, uint32_t index = 0);
#endif //HELLO_MAC_PBR_COMPONENT_H
