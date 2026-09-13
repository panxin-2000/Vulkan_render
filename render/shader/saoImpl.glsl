/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * This is our implementation of SAO -- it's not standalone because it uses materialParams
 * directly. Therefore it must be included in *.mat file that has all these parameters.
 * The main reason for using a separate file is to be able to have several version of the
 * code with only minor changes.
 */

#include "common_math.glsl"
#include "ssaoUtils.glsl"
#include "geometry.glsl"

#ifndef COMPUTE_BENT_NORMAL
#error COMPUTE_BENT_NORMAL must be set
#endif

const float kLog2LodRate = 3.0;

// Ambient Occlusion, largely inspired from:
// "The Alchemy Screen-Space Ambient Obscurance Algorithm" by Morgan McGuire
// "Scalable Ambient Obscurance" by Morgan McGuire, Michael Mara and David Luebke

vec3 tapLocation(float i, const float noise) {
    float offset = ((2.0 * PI) * 2.4) * noise;
    float angle = ((i * materialParams.sampleCount.y) * materialParams.spiralTurns) * (2.0 * PI) + offset;
    float radius = (i + noise + 0.5) * materialParams.sampleCount.y;
    return vec3(cos(angle), sin(angle), radius * radius);
}

highp vec2 startPosition(const float noise) {
    float angle = ((2.0 * PI) * 2.4) * noise;
    return vec2(cos(angle), sin(angle));
}

highp mat2 tapAngleStep() {
    highp vec2 t = materialParams.angleIncCosSin;
    return mat2(t.x, t.y, -t.y, t.x);
}

vec3 tapLocationFast(float i, vec2 p, const float noise) {
    float radius = (i + noise + 0.5) * materialParams.sampleCount.y;
    return vec3(p, radius * radius);
}

void computeAmbientOcclusionSAO(inout float occlusion,
        inout vec3 bentNormal,
        float i,
        float ssDiskRadius,
        const highp vec2 uv,
        const highp vec3 origin,
        const vec3 normal,
        const vec2 tapPosition,
        const float noise,
        mat4 Projection,
        mat4 invProjection,
        sampler2D depth_image) {

    // tap 的 意思是信号学 上的 一个概念, 用于 描述一次采样 或者 一个很尖的三角形
    vec3 tap = tapLocationFast(i, tapPosition, noise);

    float ssRadius = max(1.0, tap.z * ssDiskRadius); // at least 1 pixel screen-space radius

    //    vec2 positionParams = textureSize(depth_image, 0);

    vec2 uvSamplePos = uv + vec2(ssRadius * tap.xy) * (1.0 / textureSize(depth_image, 0));
    // 拿到了采样的坐标

    float level = clamp(floor(log2(ssRadius)) - kLog2LodRate, 0.0, float(materialParams.maxLevel));
    // 计算可能的 采样的层级

    highp float occlusionDepth = sampleDepthLinear(depth_image, uvSamplePos, level, Projection);
    // 拿到了 半球上的 点 投影到 深度贴图 上的 UV 坐标上 的  深度


    highp vec3 p = get_view_pos(uvSamplePos, occlusionDepth, invProjection);
    //  相机空间 中的 三维坐标值

    //    highp vec3 p = computeViewSpacePositionFromDepth(uvSamplePos, occlusionDepth, positionParams);

    // now we have the sample, compute AO
    highp vec3 v = p - origin;  // sample vector
    highp float vv = dot(v, v);       // squared distance
    highp float vn = dot(v, normal);  // distance * cos(v, normal)

    //    vn 是什么?  采样点在中心点法线方向上的“垂直高度”

    // discard samples that are outside of the radius, preventing distant geometry to
    // cast shadows -- there are many functions that work and choosing one is an artistic
    // decision.
    // Need to be highp to avoid imprecision on certain hardware (e.g. PowerVR)
    highp float w = sq(max(0.0, 1.0 - vv * materialParams.invRadiusSquared));

    // discard samples that are too close to the horizon to reduce shadows cast by geometry
    // not sufficently tessellated. The goal is to discard samples that form an angle 'beta'
    // smaller than 'epsilon' with the horizon. We already have dot(v,n) which is equal to the
    // sin(beta) * |v|. So the test simplifies to vn^2 < vv * sin(epsilon)^2.
    w *= step(vv * materialParams.minHorizonAngleSineSquared, vn * vn);

    float sampleOcclusion = max(0.0, vn + (abs(origin.z) * materialParams.bias)) / (vv + materialParams.peak2);
    occlusion += w * sampleOcclusion;

    // 这里的 计算稍微有点复杂 , 知道最后为了 计算 occlusion, 但是我没有搞清楚方向

    #if COMPUTE_BENT_NORMAL

    // TODO: revisit how we choose to keep the normal or not
    // reject samples beyond the far plane
    if (occlusionDepth * materialParams.invFarPlane < 1.0) {
        float rr = 1.0 / materialParams.invRadiusSquared;
        float cc = vv - vn * vn;
        float s = sqrt(max(0.0, rr - cc));
        vec3 n = normalize(v + normal * (s - vn));// vn is negative
        bentNormal += n * (sampleOcclusion <= 0.0 ? 1.0 : 0.0);
    }

    #endif
}
/**
*  out float obscurance  遮蔽度 / 遮挡度 引入了距离衰减函数  周围的障碍物距离当前像素越近，遮蔽贡献越大（阴影越黑）；距离越远，遮蔽贡献越小（阴影越淡）
*  out vec3 bentNormal   弯曲法线  它是对像素周围“哪个方向最空旷、最不容易被遮挡”的一种数学描述 , 暂时没有使用
*  highp vec2 uv         两个意思吧, 即是 深度贴图的 UV,也是输出 到 image 的 UV
*  highp vec3 origin     半球采样 的 原点
*  vec3 normal           半球采样 的 原点 上的 法线
*  ivec2 FragCoord       这里给出了 当前的 frag 阶段的 屏幕坐标 或者 compute 阶段 的 计算坐标, 用于计算之后的 扰动噪声
*  mat4 Projection       投影矩阵
*  mat4 invProjection    逆投影矩阵
*  sampler2D depth_image 需要进行采样的深度贴图
**/
void scalableAmbientObscurance(out float obscurance,
        out vec3 bentNormal,  // 弯曲法线  它是对像素周围“哪个方向最空旷、最不容易被遮挡”的一种数学描述
        highp vec2 uv,
        highp vec3 origin,
        vec3 normal,
        ivec2 FragCoord,
        mat4 Projection,
        mat4 invProjection,
        sampler2D depth_image) {
    float noise = interleavedGradientNoise(FragCoord);
    highp vec2 tapPosition = startPosition(noise);
    highp mat2 angleStep = tapAngleStep();

    // Choose the screen-space sample radius
    // proportional to the projected area of the sphere
    float ssDiskRadius = -(materialParams.projectionScaleRadius / origin.z);

    obscurance = 0.0;
    bentNormal = normal;
    for (float i = 0.0; i < materialParams.sampleCount.x; i += 1.0) {
        computeAmbientOcclusionSAO(obscurance, bentNormal,
                i, ssDiskRadius, uv, origin, normal, tapPosition, noise, Projection, invProjection, depth_image);
        tapPosition = angleStep * tapPosition;
    }
    obscurance = sqrt(obscurance * materialParams.intensity);
    #if COMPUTE_BENT_NORMAL
    bentNormal = normalize(bentNormal);
    #endif
}
