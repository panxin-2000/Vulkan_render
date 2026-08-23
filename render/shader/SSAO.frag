#version 450

#extension GL_EXT_nonuniform_qualifier: require
#extension GL_GOOGLE_include_directive: enable


#define COMPUTE_BENT_NORMAL 0
layout (constant_id = 0) const int SSAO_KERNEL_SIZE = 64;
layout (constant_id = 1) const float SSAO_RADIUS = 0.5;


#include "global_shader_common.glsl"

#include "geometry.glsl"

layout (location = 0) in vec2 inUV;
layout (location = 0) out float out_occlusion_R8_UNORM; // 这里更改了,

float interleavedGradientNoise(vec2 fragCoord) {
    return fract(52.582f * fract(fragCoord.x * 0.06711f + fragCoord.y * 0.00583f));
}


const vec3 SSAO_KERNEL[64] = vec3[64](
        vec3(0.0121, 0.0084, 0.0031), vec3(0.0234, -0.0152, 0.0076), vec3(-0.0315, 0.0211, 0.0124), vec3(0.0412, 0.0321, 0.0185),
        vec3(-0.0154, -0.0421, 0.0231), vec3(0.0512, -0.0124, 0.0312), vec3(0.0215, 0.0542, 0.0384), vec3(-0.0612, 0.0312, 0.0421),
        vec3(0.0321, -0.0654, 0.0512), vec3(-0.0712, -0.0215, 0.0594), vec3(0.0815, 0.0412, 0.0684), vec3(-0.0421, 0.0821, 0.0754),
        vec3(0.0912, -0.0512, 0.0831), vec3(-0.0615, -0.0942, 0.0921), vec3(0.1012, 0.0612, 0.1042), vec3(-0.1115, 0.0342, 0.1124),
        vec3(0.0521, -0.1142, 0.1231), vec3(-0.1254, 0.0712, 0.1324), vec3(0.1312, 0.0821, 0.1412), vec3(-0.0915, -0.1342, 0.1512),
        vec3(0.1415, -0.1042, 0.1621), vec3(-0.1512, -0.1142, 0.1742), vec3(0.1615, 0.0912, 0.1834), vec3(-0.1712, 0.1215, 0.1924),
        vec3(0.1815, -0.1312, 0.2042), vec3(-0.1912, 0.1415, 0.2184), vec3(0.2015, -0.1512, 0.2294), vec3(-0.2112, -0.1615, 0.2412),
        vec3(0.2215, 0.1712, 0.2542), vec3(-0.2312, 0.1815, 0.2684), vec3(0.2415, -0.1912, 0.2812), vec3(-0.2512, -0.2015, 0.2984),
        vec3(0.2615, 0.2112, 0.3142), vec3(-0.2712, 0.2215, 0.3294), vec3(0.2815, -0.2312, 0.3442), vec3(-0.2912, -0.2415, 0.3612),
        vec3(0.3015, 0.2512, 0.3784), vec3(-0.3112, 0.2615, 0.3954), vec3(0.3215, -0.2712, 0.4124), vec3(-0.3312, -0.2815, 0.4312),
        vec3(0.3415, 0.2912, 0.4494), vec3(-0.3512, 0.3015, 0.4684), vec3(0.3615, -0.3112, 0.4892), vec3(-0.3712, -0.3215, 0.5094),
        vec3(0.3815, 0.3312, 0.5312), vec3(-0.3912, 0.3415, 0.5524), vec3(0.4015, -0.3512, 0.5754), vec3(-0.4112, -0.3615, 0.5984),
        vec3(0.4215, 0.3712, 0.6212), vec3(-0.4312, 0.3815, 0.6454), vec3(0.4415, -0.3912, 0.6694), vec3(-0.4512, -0.4015, 0.6942),
        vec3(0.4615, 0.4112, 0.7194), vec3(-0.4712, 0.4215, 0.7454), vec3(0.4815, -0.4312, 0.7712), vec3(-0.4912, -0.4415, 0.7984),
        vec3(0.5015, 0.4512, 0.8264), vec3(-0.5112, 0.4615, 0.8544), vec3(0.5215, -0.4712, 0.8834), vec3(-0.5312, -0.4815, 0.9132),
        vec3(0.5415, 0.4912, 0.9434), vec3(-0.5512, 0.5015, 0.9742), vec3(0.5615, -0.5112, 1.0000), vec3(-0.5712, -0.5215, 1.0000)
);



vec3 get_view_pos(vec2 uv, float depth, mat4 invProjection){
    vec4 clipPos = vec4(uv * 2.0 - 1.0, depth, 1.0);
    // 3. 乘以投影矩阵的逆矩阵，变换回 View 空间（裁剪空间逆变换）
    vec4 viewPos = invProjection * clipPos;
    // 4. 执行透视除法（Perspective Divide），此时 viewPos.z 就是 View 空间的深度
    viewPos /= viewPos.w;
    return viewPos.xyz;
}

highp vec3 computeViewSpaceNormalHighQ_temp(
        const highp sampler2D depthTexture, const highp vec2 uv,
        const highp float depth, const highp vec3 position,
        highp vec2 texel, highp mat4 invProjection) {
    precision highp
    float;

    vec3 pos_c = position;
    highp vec2 dx = vec2(texel.x, 0.0);
    highp vec2 dy = vec2(0.0, texel.y);

    vec4 H;
    H.x = sampleDepth(depthTexture, uv - dx, 0.0);
    H.y = sampleDepth(depthTexture, uv + dx, 0.0);
    H.z = sampleDepth(depthTexture, uv - dx * 2.0, 0.0);
    H.w = sampleDepth(depthTexture, uv + dx * 2.0, 0.0);
    vec2 he = abs((2.0 * H.xy - H.zw) - depth);
    vec3 pos_l = get_view_pos(uv - dx, (H.x), invProjection);
    vec3 pos_r = get_view_pos(uv + dx, (H.y), invProjection);
    vec3 dpdx = (he.x < he.y) ? (pos_c - pos_l) : (pos_r - pos_c);

    vec4 V;
    V.x = sampleDepth(depthTexture, uv - dy, 0.0);
    V.y = sampleDepth(depthTexture, uv + dy, 0.0);
    V.z = sampleDepth(depthTexture, uv - dy * 2.0, 0.0);
    V.w = sampleDepth(depthTexture, uv + dy * 2.0, 0.0);
    vec2 ve = abs((2.0 * V.xy - V.zw) - depth);
    vec3 pos_d = get_view_pos(uv - dy, (V.x), invProjection);
    vec3 pos_u = get_view_pos(uv + dy, (V.y), invProjection);
    vec3 dpdy = (ve.x < ve.y) ? (pos_c - pos_d) : (pos_u - pos_c);

    return normalize(cross(dpdy, dpdx));
}


void main()
{

    highp float depth = sampleDepth(global_depth, inUV, 0.0);
    vec3 viewPos = get_view_pos(inUV, depth, invProjection);

    //    highp float z = linearizeDepth(depth);
    //    vec2 positionParams = vec2(invProjection[0][0] * 2, invProjection[1][1] * 2);
    //    highp vec3 origin = computeViewSpacePositionFromDepth(inUV, z, positionParams);

    vec3 normal = computeViewSpaceNormalHighQ_temp(global_depth, inUV, viewPos.z, viewPos, 1 / (screen_size.xy),
            invProjection);

    float noise = interleavedGradientNoise(gl_FragCoord.xy);
    float angle = noise * 2.0 * 3.1415926535f; // 映射到 0 到 360 度
    vec3 randomVec = vec3(cos(angle), sin(angle), 0.0); // 完美的二维随机切线


    //    ivec2 texDim = textureSize(global_depth, 0);
    //    ivec2 noiseDim = textureSize(ssaoNoise, 0);
    //    const vec2 noiseUV = vec2(float(texDim.x) / float(noiseDim.x), float(texDim.y) / (noiseDim.y)) * inUV;
    //    vec3 randomVec = texture(ssaoNoise, noiseUV).xyz * 2.0 - 1.0;

    // Create TBN matrix
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(tangent, normal);
    mat3 TBN = mat3(tangent, bitangent, normal);

    // Calculate occlusion value
    float occlusion = 0.0f;
    // remove banding
    const float bias = 0.025f;
    for (int i = 0; i < SSAO_KERNEL_SIZE; i++)
    {
        vec3 samplePos = TBN * SSAO_KERNEL[i];
        samplePos = viewPos + samplePos * SSAO_RADIUS;

        // project
        vec4 offset = vec4(samplePos, 1.0f);
        offset = projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5f + 0.5f;

        highp float depth_temp = sampleDepth(global_depth, offset.xy, 0.0);

        vec3 sampleDepth = get_view_pos(offset.xy, depth_temp, invProjection);


        float rangeCheck = smoothstep(0.0f, 1.0f, SSAO_RADIUS / abs(viewPos.z - sampleDepth.z));
        occlusion += (sampleDepth.z >= samplePos.z + bias ? 1.0f : 0.0f) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / float(SSAO_KERNEL_SIZE));



    out_occlusion_R8_UNORM = occlusion;
}
