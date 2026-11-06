//
// Created by 潘鑫 on 2026/3/13.
//


#extension GL_GOOGLE_include_directive: enable
#extension GL_EXT_nonuniform_qualifier: require

#include "commom_function_and_struct.glsl"


layout (set = 0, binding = 0) uniform sampler bindless_samplers[512];
layout (set = 0, binding = 4) uniform texture2D bindless_texture2D[]; // 不固定数量多只能分配到末尾
// layout (set = 0, binding = 0) uniform texture2D bindless_Textures[];


#define SHADOW_MAP_CASCADE_COUNT 4


layout (set = 1, binding = 0) uniform global_parameters
{
    mat4 view;
    mat4 projection;
    mat4 invView;
    mat4 invProjection;
    mat4 inv_VP;
    mat4 cascadeViewProjMat[SHADOW_MAP_CASCADE_COUNT];
    vec4 frustum_planes[6];
    vec4 light_frustum_planes[4 * 6];
    vec3 viewPos; // camPos
    Light light;
    vec4 screen_size;
    vec4 mouse_position;
    vec4 cascadeSplits;
    float film_grain_intensity;
    float camera_vignette_smoothness;
    float camera_vignette_intensity;
    uint render_timeline;
    float aperture;          // 光圈大小 (控制虚化强度)
    float focalLength;       // 镜头焦距 (控制视野和虚化程度)
    float sensorWidthMeters; // 全画幅 35mm  输入值 0.035
    float MaxBlurPixels;
    float fogStart;
    float fogEnd;
    float fogDensity;
    uint fogType;
    float fxaaQualitySubpix;
    float fxaaQualityEdgeThreshold;
    float fxaaQualityEdgeThresholdMin;
    float fxaa_pad;

    vec4 fogColor;

    SphericalHarmonics SH;
    vec4 pad[6];
};



layout (set = 1, binding = 1) readonly buffer global_PBR_parameters {
    ShaderMaterial material[];
};



layout (set = 1, binding = 2) uniform sampler2D global_offscreen;
layout (set = 1, binding = 3) uniform sampler2D global_Blur_SSAO;       // 这里是第一次输出的结果
layout (set = 1, binding = 4) uniform sampler2D global_depth;
layout (set = 1, binding = 5) uniform sampler2DArray global_shadow_texture;  // 这里是经过模糊之后的SSAO
layout (set = 1, binding = 6) uniform sampler2D global_MSDF_texture;  // 这里是经过模糊之后的SSAO
layout (set = 1, binding = 7) readonly buffer global_model_matrix {
    mat4 model_matrix[];
};

layout (set = 1, binding = 8) readonly buffer global_entity_to_screen {
    uint entities[];
};

layout (set = 1, binding = 9) readonly buffer global_material_parameters {
    uint material_pbr_index[];
};


//layout(rgba16f, set = 1, binding = 6)  uniform image2D global_compute_Texture;
//layout (r8, set = 1, binding = 7) uniform image2D global_SSAO_compute_write;       // 这里是第一次输出的结果
//layout (r8, set = 1, binding = 8) uniform image2D global_Blur_SSAO_compute_write;       // 这里是第一次输出的结果
//layout (rgba16f, set = 1, binding = 9) uniform image2D global_dof_BlurImg;       // 这里是第一次输出的结果
//layout (rgba16f, set = 1, binding = 10) uniform image2D global_dof_result;       // 这里是第一次输出的结果

float get_roughness(ShaderMaterial material) {
    return material.roughnessFactor;
}
float get_metallic(ShaderMaterial material) {
    return material.metallicFactor;
}

vec4 get_base_color(ShaderMaterial material, vec2 inUV)
{
    vec4 inColor = texture(sampler2D(bindless_texture2D[material.baseColorTexture], bindless_samplers[0]), inUV);
    return inColor * material.baseColorFactor;
}

vec4 get_emissive_color(ShaderMaterial material, vec2 inUV)
{
    vec4 emissive = texture(sampler2D(bindless_texture2D[material.emissiveTexture], bindless_samplers[0]), inUV);
    return emissive * material.emissiveFactor;
}

float get_Roughness(ShaderMaterial material, vec2 inUV)
{
    // Occlusion, Roughness, Metallic
    vec4 Color = texture(sampler2D(bindless_texture2D[material.ORM_Texture], bindless_samplers[0]), inUV);
    return Color.g * material.roughnessFactor;
}

float get_Metallic(ShaderMaterial material, vec2 inUV)
{
    // Occlusion, Roughness, Metallic
    vec4 Color = texture(sampler2D(bindless_texture2D[material.ORM_Texture], bindless_samplers[0]), inUV);
    return Color.b * material.metallicFactor;
}

float get_Occlusion(ShaderMaterial material, vec2 inUV)
{
    // Occlusion, Roughness, Metallic
    vec4 Color = texture(sampler2D(bindless_texture2D[material.ORM_Texture], bindless_samplers[0]), inUV);
    return 1.0 + material.occlusionStrength + (Color.r - 1.0);
}

vec3 get_Occlusion_Roughness_Metallic(ShaderMaterial material, vec2 inUV) {
    // Occlusion, Roughness, Metallic
    vec4 Color = texture(sampler2D(bindless_texture2D[material.ORM_Texture], bindless_samplers[0]), inUV);
    vec3 result = Color.rgb * vec3(material.occlusionStrength, material.roughnessFactor, material.metallicFactor);
    result.r = 1.0 + result.r - material.occlusionStrength;
    return result;
}

vec4 getScreenAABBUV(AABB_box box, inout float boxClosestDepth) {
    // 1. 计算 3D 空间下的最小点和最大点
    vec3 c = box.centroid_points.xyz;
    vec3 e = box.direction_intervals.xyz;
    vec3 boxMin = c - e;
    vec3 boxMax = c + e;

    // 2. 构造 AABB 的 8 个世界/局部空间顶点
    vec3 vertices[8];
    vertices[0] = vec3(boxMin.x, boxMin.y, boxMin.z);
    vertices[1] = vec3(boxMax.x, boxMin.y, boxMin.z);
    vertices[2] = vec3(boxMin.x, boxMax.y, boxMin.z);
    vertices[3] = vec3(boxMax.x, boxMax.y, boxMin.z);
    vertices[4] = vec3(boxMin.x, boxMin.y, boxMax.z);
    vertices[5] = vec3(boxMax.x, boxMin.y, boxMax.z);
    vertices[6] = vec3(boxMin.x, boxMax.y, boxMax.z);
    vertices[7] = vec3(boxMax.x, boxMax.y, boxMax.z);

    // 初始化 UV 的极值
    // 使用大数值初始化 min，小数值初始化 max 方便后续用 min/max 函数迭代
    vec2 minUV = vec2(1.0);
    vec2 maxUV = vec2(0.0);

    // 3. 遍历 8 个顶点进行投影计算
    for (int i = 0; i < 8; i++) {
        // 变换到裁剪空间 (Clip Space)
        vec4 clipSpacePos = projection * view * vec4(vertices[i], 1.0);

        // 归一化设备坐标 (NDC) 变换
        // 乘以 0.5 + 0.5 是为了将 NDC 的 [-1, 1] 映射到屏幕 UV 的 [0, 1] 空间
        vec3 ndcPos = clipSpacePos.xyz / clipSpacePos.w;
        vec2 uv = ndcPos.xy * 0.5 + vec2(0.5);

        // 裁剪保护：防止物体出屏或在相机背面时造成 UV 严重畸变
        uv = clamp(uv, vec2(0.0), vec2(1.0));

        // 迭代更新屏幕 AABB 的边界
        minUV = min(minUV, uv);
        maxUV = max(maxUV, uv);
        boxClosestDepth = max(boxClosestDepth, ndcPos.z);
        // 上面用的是 max 是因为  反转了 z 轴
    }

    // 4. 组合成上一个函数 sampleHZB 需要的 vec4 (aabbUV)
    // 映射关系：x=min_u, y=min_v, z=max_u, w=max_v
    return vec4(minUV.x, minUV.y, maxUV.x, maxUV.y);
}


// Calculation of TBN matrix and terminology based on "Surface
// Gradient-Based Bump Mapping Framework" (2020)











