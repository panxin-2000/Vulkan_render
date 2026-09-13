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

#ifndef FILAMENT_MATERIALS_DEPTH_UTILS
#define FILAMENT_MATERIALS_DEPTH_UTILS

//highp float linearizeDepth(highp float depth)
//{
//    // Our far plane is at infinity, which causes a division by zero below, which in turn
//    // causes some issues on some GPU. We workaround it by replacing "infinity" by the closest
//    // value representable in  a 24 bit depth buffer.
//    const highp float preventDiv0 = 1.0 / 16777216.0;
//    mat4 p = invProjection;
//    // this works with perspective and ortho projections, for a perspective projection
//    // this resolves to -near/depth, for an ortho projection this resolves to depth*(far - near) - far
//    return (depth * p[2].z + p[3].z) / max(depth * p[2].w + p[3].w, preventDiv0);
//}
// 这里返回的 也是z 轴, 也是负数
highp float linearizeDepth(highp float depth, mat4 Projection)
{
    // 直接提取矩阵中掌管 NDC 到 ViewSpace 深度缩放与偏移的两大核心系数
    float P22 = EIGEN_INDEX(Projection, 2, 2);
    float P32 = EIGEN_INDEX(Projection, 3, 2);
    // 不管是常规 Z 还是 Reversed-Z，甚至包括“无限远景 Reversed-Z”，
    // 它们在代数化简后，在 GPU 硬件层面全都能完美收敛到这个极简的分式中！
    return P32 / (depth - P22);
}


vec3 get_view_pos(vec2 uv, float depth, mat4 invProjection){
    vec4 clipPos = vec4(uv * 2.0 - 1.0, depth, 1.0);
    float x = invProjection[0][0] * clipPos.x;
    float y = invProjection[1][1] * clipPos.y;
    float z = EIGEN_INDEX(invProjection,2,2) * clipPos.z + EIGEN_INDEX(invProjection,2,3);
    float w = EIGEN_INDEX(invProjection,3,2) * clipPos.z + EIGEN_INDEX(invProjection,3,3);
    return vec3(x, y, z) / w;
}



// 传入物理近平面 near 和远平面 far
//float LinearizeDepth(float z_ndc, float near, float far) {
//    return (near * far) / (far - z_ndc * (far - near));
//}


highp float sampleDepth(const highp sampler2D depthTexture, const highp vec2 uv, float lod) {
    return textureLod(depthTexture, (uv), lod).r;
}

highp float sampleDepthLinear(const highp sampler2D depthTexture,
        const highp vec2 uv, float lod, mat4 Projection) {
    return linearizeDepth(sampleDepth(depthTexture, uv, lod), Projection);
}

#endif // #define FILAMENT_MATERIALS_DEPTH_UTILS

