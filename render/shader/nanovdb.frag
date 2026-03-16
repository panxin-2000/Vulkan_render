#version 450


#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"


#extension GL_EXT_shader_explicit_arithmetic_types_int64: require
#include "PNanoVDB.h"
// 确保路径正确

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outColor;

// 绑定存储 NanoVDB 数据的 VkBuffer
layout (set = 0, binding = 0, std430) readonly buffer VDBBuffer {
    uint64_t data[];
} vdb;

// 摄像机参数
layout (push_constant) uniform PushConstants {
    mat4 invViewProj;
    vec3 camPos;
    float stepSize; // 步长，越小质量越高但越慢
} pc;

void main() {
    // 1. 初始化射线 (从屏幕坐标还原世界坐标射线)
    vec4 target = pc.invViewProj * vec4(inUV * 2.0 - 1.0, 1.0, 1.0);
    vec3 rayDir = normalize(target.xyz / target.w - pc.camPos);

    // 2. 初始化 NanoVDB 访问器
    pNanovdb_pGrid_t grid = pNanovdb_pGrid_t(vdb.data);
    pNanovdb_ReadAccessor_t acc;
    pNanovdb_ReadAccessor_init(acc, grid);

    // 3. 获取 VDB 包围盒 (世界空间)
    vec3 bMin = pNanovdb_grid_index_to_world(grid, pNanovdb_grid_bbox_min(grid));
    vec3 bMax = pNanovdb_grid_index_to_world(grid, pNanovdb_grid_bbox_max(grid));

    // 4. 计算射线与包围盒的交点 (AABB Intersection)
    float tNear, tFar;
    bool intersect = intersectAABB(pc.camPos, rayDir, bMin, bMax, tNear, tFar);

    if (!intersect) {
        outColor = vec4(0, 0, 0, 1); // 未击中包围盒
        return;
    }

    // 5. Raymarching 主循环
    float t = max(tNear, 0.0);
    float absorption = 1.0; // 剩余光强度
    vec3 finalColor = vec3(0.0);
    float densityScale = 50.0; // 密度缩放，控制烟雾厚度

    for (int i = 0; i < 256; ++i) { // 限制最大步数防止超时
                                    if (t > tFar || absorption < 0.01) break;

                                    vec3 worldPos = pc.camPos + t * rayDir;

                                    // 关键步骤：将世界坐标转为 VDB 索引坐标并采样
                                    vec3 indexPos = pNanovdb_grid_world_to_indexf(grid, worldPos);
                                    float density = pNanovdb_read_float(grid, acc, indexPos);

                                    if (density > 0.0) {
                                        // 计算局部不透明度 (Beer's Law 简化版)
                                        float localOpacity = density * densityScale * pc.stepSize;

                                        // 累积颜色 (这里简单设为白色，你可以根据 density 采样 ColorRamp)
                                        finalColor += absorption * localOpacity * vec3(1.0);

                                        // 更新剩余光强
                                        absorption *= max(0.0, 1.0 - localOpacity);
                                    }

                                    t += pc.stepSize;
    }

    outColor = vec4(finalColor, 1.0 - absorption);
}

// 辅助函数：射线与 AABB 求交
bool intersectAABB(vec3 ro, vec3 rd, vec3 bMin, vec3 bMax, out float t0, out float t1) {
    vec3 invR = 1.0 / rd;
    vec3 tbot = invR * (bMin - ro);
    vec3 ttop = invR * (bMax - ro);
    vec3 tmin = min(tbot, ttop);
    vec3 tmax = max(tbot, ttop);
    t0 = max(tmin.x, max(tmin.y, tmin.z));
    t1 = min(tmax.x, min(tmax.y, tmax.z));
    return t0 <= t1 && t1 > 0.0;
}
