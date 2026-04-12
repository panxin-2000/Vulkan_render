// 需要 与 PBR_component 布局相同
struct ShaderMaterial {
    vec4 baseColorFactor;
    vec4 emissiveFactor;

    float metallicFactor;
    float roughnessFactor;
    float occlusionStrength;
    float alphaCutoff;

    uint doubleSided; // 是否开始背面剪裁， 叶子、旗帜、纸张等超薄物体 需要为 true
    uint alphaMode;           // 有三个值
    //                              OPAQUE (不透明 - 默认)
    //                              MASK  基于 alphaCutoff 阈值进行“全有或全无”的硬切
    //                              BLEND (混合/半透明)
    // 需要一个为全为一的贴图，也就是纯白的贴图
    uint baseColorTexture; // 基础颜色 贴图
    uint normalTexture; //

    uint emissiveTexture; // 自发光 贴图
    uint ORM_Texture; // Occlusion, Roughness, Metallic

    uint pad_1;
    uint pad_2;
    // 下面这两个有什么用？
    //    vec4 diffuseFactor;
    //    vec4 specularFactor;
    // 那么总共的字节数 是 16 + 16 + 16 + 16 + 16  = 80 字节
};


struct PBRInfo
{
    float NdotL;                  // cos angle between normal and light direction
    float NdotV;                  // cos angle between normal and view direction
    float NdotH;                  // cos angle between normal and half vector
    float LdotH;                  // cos angle between light direction and half vector
    float VdotH;                  // cos angle between view direction and half vector
    float perceptualRoughness;    // roughness value, as authored by the model creator (input to shader)
    float metalness;              // metallic value at the surface
    vec3 reflectance0;            // full reflectance color (normal incidence angle)
    vec3 reflectance90;           // reflectance color at grazing angle
    float alphaRoughness;         // roughness mapped to a more linear change in the roughness (proposed by [2])
    vec3 diffuseColor;            // color contribution from diffuse lighting
    vec3 specularColor;           // color contribution from specular lighting
};