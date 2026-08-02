#version 450 core
#extension GL_EXT_shader_explicit_arithmetic_types_int64: enable
#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"



layout (location = 0) out vec4 outFragColor_B8G8R8A8_SRGB;

layout (set = 2, binding = 0) uniform model_4x4
{
    mat4 model;
};

layout (set = 2, std430, binding = 1) readonly buffer nanovdb_buffer {
    uint raw_data[];
} vdb_ssbo;


layout (set = 2, std140, binding = 2) readonly buffer light_buffer {
    Light lights[];
};

layout (set = 2, binding = 3) uniform nanovdb_size
{
    uint vdb_size;
};

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inLightVec;
layout (location = 3) in vec3 inViewVec;
layout (location = 4) in vec4 inShadowCoord;
layout (location = 5) in vec3 inWorldPos;

// 2. 核心：将库内部访问宏指向这个数组
#define pnanovdb_buf_data vdb_ssbo.raw_data

#define PNANOVDB_GLSL
#define PNANOVDB_ADDRESS_32
#include "PNanoVDB.h"
#include "PNanoVDB_distance.glsl"
#include "VdbCommon.glsl"




bool trace_vdb_is_hit(VdbSampler vdb_sampler,
                      pnanovdb_vec3_t view_position,
                      pnanovdb_vec3_t view_direction, inout float t_min, inout float t_max) {

    // 只要你拿到了其中一个网格的地址，调用该函数都能得到整个缓冲区包含的网格总数
    //    pnanovdb_uint32_t grid_count = pnanovdb_grid_get_grid_count(nanovdb_buffer, Grid);
    //    if (grid_count > pnanovdb_uint32_t(1)) {
    //        // 拿到第二个的
    //        pnanovdb_uint64_t next_size = pnanovdb_grid_get_grid_size(nanovdb_buffer, Grid);
    //        pnanovdb_grid_handle_t Grid_2;
    //        pnanovdb_address_t address_grid_2;
    //        address_grid_2.byte_offset = 0;
    //        Grid_2.address = address_grid_2;
    //    }

    // 1. 初始化 Buffer 和 Grid 地址
    // 注意：size_in_words 填入实际大小，或者如果是指针访问模式，填入一个足够大的占位值
    // 这里是创建一个 pnanovdb_buf_t 的方式， 给出地址和最大的大小，在需要检查边界时才最使用最大的大小
    // pnanovdb_buf_t buf;// = pnanovdb_make_buf(nanovdb_buffer.data, nanovdb_size);

    // 3. 坐标转换：将世界空间射线转到索引空间
    // HDDA 必须在索引空间（Index Space）运行
    pnanovdb_vec3_t origin_index = pnanovdb_grid_world_to_indexf(vdb_sampler.GridBuffer, vdb_sampler.Grid, view_position);
    pnanovdb_vec3_t direction_index = pnanovdb_grid_world_to_index_dirf(vdb_sampler.GridBuffer, vdb_sampler.Grid, view_direction);

    float hit_value; // 撞击点处的值（通常接近 0）
    float t_hit = 0.0f; // 输出：撞击时的 t 值（相对于 index_p）
    bool is_hit = pnanovdb_hdda_zero_crossing(vdb_sampler.GridType,
                                              vdb_sampler.GridBuffer,
                                              vdb_sampler.Accessor, // 用于加速的结构
                                              origin_index,
                                              t_min,
                                              direction_index,
                                              t_max,
                                              t_hit, // 这里才是能返回结果的内容
                                              hit_value  // 击中时的 float 的值
    );
    if (is_hit) {
        pnanovdb_vec3_t hit_pos_index = pnanovdb_hdda_ray_start(origin_index, t_hit, direction_index);
    }
    return is_hit;

}


//if (is_hit) {
//
//
//float distance_value = vdb_get_out_distance_same_density(grid_type,
//buf,
//Accessor, // 用于加速的结构
//hit_pos_index,
//t_min,
//direction_index,
//t_max);  // AABB 包围盒的对角线长度 ，单步的距离
//float T = exp(-distance_value * sigma_a);
//}
//discard;
//
//return 0.0;


// grid_class
// PNANOVDB_GRID_CLASS_LEVEL_SET 1		// narrow band levelset, e.g. SDF
// PNANOVDB_GRID_CLASS_FOG_VOLUME 2	// fog volume, e.g. density
// PNANOVDB_GRID_CLASS_STAGGERED 3		// staggered MAC grid, e.g. velocity
// PNANOVDB_GRID_CLASS_POINT_INDEX 4	// point index grid
// PNANOVDB_GRID_CLASS_POINT_DATA 5	// point data grid
// PNANOVDB_GRID_CLASS_TOPOLOGY 6		// grid with active states only (no values)
// PNANOVDB_GRID_CLASS_VOXEL_VOLUME 7	// volume of geometric cubes, e.g. minecraft
vec3 check_grid_class(pnanovdb_uint32_t grid_index, pnanovdb_uint32_t grid_class) {
    pnanovdb_buf_t buf;
    pnanovdb_grid_handle_t Grid;
    pnanovdb_address_t address;
    address.byte_offset = 0;
    Grid.address = address;
    pnanovdb_uint32_t grid_class_read = pnanovdb_grid_get_grid_class(buf, Grid);
    if (grid_class_read == grid_class) {
        return vec3(0, 1.0, 0);
    } else {
        return vec3(1.0, 0, 0);
    }

}

void main() {
    vec2 screen_UV = gl_FragCoord.xy / screen_size.xy;//如何用这个来替代呢？ screen_UV 在0到1之间
    vec2 ndc = screen_UV * 2.0 - 1.0;
    vec4 viewTarget = inv_VP * vec4(ndc, 0.2, 1.0);
    vec3 far_point = viewTarget.xyz / viewTarget.w;
    vec3 rayDir = normalize(far_point - viewPos);

    mat3 modelRot = mat3(model);
    mat3 invModelRot = transpose(modelRot); // 若有不等比缩放则改用 inverse(modelRot)
    vec3 localRayDir = invModelRot * rayDir;


    vec3 model_world_pos = model[3].xyz;
    //    vec3 rayOrigin = viewPos - model_world_pos.xyz;
    vec3 rayOrigin = viewPos - model_world_pos;

    vec3 localRayOrigin = invModelRot * rayOrigin;

    pnanovdb_buf_t buf; // = pnanovdb_make_buf(ptr, size / 4);

    // 射线的方向可以 由 观察点 和 UV 坐标计算得出
    pnanovdb_vec3_t world_p = pnanovdb_vec3_t(rayOrigin);
    pnanovdb_vec3_t world_d = pnanovdb_vec3_t(rayDir);
    float tmax = 1000;
    float tmin = 0;
    VdbSampler VdbSampler = InitVdbSampler(buf);



    bool is_hit = trace_vdb_is_hit(VdbSampler, world_p, world_d, tmin, tmax);
    if (is_hit == true) {
        //        float sigma_a = 0.001;
        //        float density = 1;
        //
        //        float distance_value = vdb_get_out_distance_same_density(grid_type,
        //                                                                 buf,
        //                                                                 Accessor, // 用于加速的结构
        //                                                                 hit_pos_index,
        //                                                                 t_min,
        //                                                                 direction_index,
        //                                                                 t_max);  // AABB 包围盒的对角线长度 ，单步的距离
        //        //        float T = exp(-distance_value * sigma_a);
        //
        //
        //        float T = trace_vdb_transmission(buf, world_p, world_d, tmin, tmax, sigma_a, density);
        //        vec3 volume_color = vec3(1.0, 1.0, 1.0);
        //        outFragColor_B8G8R8A8_SRGB = vec4(volume_color, 1 - T);
        //
        //        float sigma_a = 0.001;
        //        float T = exp(-distance * sigma_a);

        outFragColor_B8G8R8A8_SRGB = vec4(1.0, 1.0, 1.0, 1.0);
        // 前景色 * alpha + 背景色 * (1 - alpha)
    } else {
        discard;
        // 不相交的时候就忽略当前像素的颜色
    }
}
