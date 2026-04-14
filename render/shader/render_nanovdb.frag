#version 450 core
#extension GL_EXT_shader_explicit_arithmetic_types_int64: enable
#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"

layout (location = 0) out vec4 outFragColor_B8G8R8A8_SRGB;


layout (set = 2, std430, binding = 1) readonly buffer nanovdb_buffer {
    uint raw_data[];
} vdb_ssbo;

layout (set = 2, binding = 2) uniform nanovdb_size
{
    uint size;
};

struct Light {
    vec3 color;
    vec4 position;
    float radius;
};

layout (set = 2, std140, binding = 3) readonly buffer light_buffer {
    Light lights[];
};

layout (location = 0) in vec2 in_UV;


// 2. 核心：将库内部访问宏指向这个数组
#define pnanovdb_buf_data vdb_ssbo.raw_data

#define PNANOVDB_GLSL
#define PNANOVDB_ADDRESS_32
#include "PNanoVDB.h"


bool trace_nanovdb_levelset(pnanovdb_buf_t nanovdb_buffer,
                            pnanovdb_vec3_t world_p,
                            pnanovdb_vec3_t world_d,
                            float tmin,
                            float tmax) {
    pnanovdb_grid_handle_t Grid;
    pnanovdb_readaccessor_t Accessor;
    pnanovdb_root_handle_t Root;

    pnanovdb_address_t address;
    address.byte_offset = 0;
    Grid.address = address;

    pnanovdb_tree_handle_t tree = pnanovdb_grid_get_tree(nanovdb_buffer, Grid);
    Root = pnanovdb_tree_get_root(nanovdb_buffer, tree);
    pnanovdb_readaccessor_init(Accessor, Root);
    pnanovdb_uint32_t grid_type = pnanovdb_grid_get_grid_type(nanovdb_buffer, Grid);

    // 只要你拿到了其中一个网格的地址，调用该函数都能得到整个缓冲区包含的网格总数
    pnanovdb_uint32_t grid_count = pnanovdb_grid_get_grid_count(nanovdb_buffer, Grid);
    if (grid_count > 1) {
        // 拿到第二个的
        pnanovdb_uint64_t next_size = pnanovdb_grid_get_grid_size(nanovdb_buffer, Grid);
        pnanovdb_grid_handle_t Grid_2;
        pnanovdb_address_t address_grid_2;
        address_grid_2.byte_offset = 0;
        Grid_2.address = address_grid_2;
    }

    // 1. 初始化 Buffer 和 Grid 地址
    // 注意：size_in_words 填入实际大小，或者如果是指针访问模式，填入一个足够大的占位值
    // 这里是创建一个 pnanovdb_buf_t 的方式， 给出地址和最大的大小，在需要检查边界时才最使用最大的大小
    pnanovdb_buf_t buf;//= pnanovdb_make_buf(nanovdb_buffer.data, 0xFFFFFFFF);


    // 3. 坐标转换：将世界空间射线转到索引空间
    // HDDA 必须在索引空间（Index Space）运行
    pnanovdb_vec3_t index_p = pnanovdb_grid_world_to_indexf(buf, Grid, world_p);
    pnanovdb_vec3_t index_d = pnanovdb_grid_world_to_index_dirf(buf, Grid, world_d);

    // 5. 准备输出参数
    pnanovdb_vec3_t hit_ijk; // 撞击点所在的体素索引坐标
    float hit_value; // 撞击点处的值（通常接近 0）
    float t_hit = 0.0f; // 输出：撞击时的 t 值（相对于 index_p）

    // 6. 执行 HDDA Zero Crossing 调用
    // 该函数会沿着射线步进，寻找符号变化（正负交替）的点

    float v = 0;
    bool is_hit = pnanovdb_hdda_zero_crossing(grid_type, buf, Accessor, index_p, tmin, index_d, tmax, t_hit, v);

    if (is_hit) {
        // 如果需要世界空间下的交点：
        // world_hit = world_p + world_d * (t_hit / length(index_d))
        // 或者直接调用 pnanovdb_grid_index_to_worldf(buf, grid_addr, hit_ijk)
        return true;
    }

    return false;
}

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
    vec2 ndc = in_UV * 2.0 - 1.0;

    // 2. 计算视图空间中的目标点 (设 z=1 为远裁剪面方向)
    vec4 viewTarget = invProjection * vec4(ndc, 1.0, 1.0);
    vec3 viewDir = viewTarget.xyz / viewTarget.w;

    // 3. 转换到世界空间
    // 这里 target.xyz / target.w 是世界空间的方向向量
    vec3 rayDir = normalize((invView * vec4(viewDir, 0.0)).xyz);

    vec3 rayOrigin = viewPos;

    pnanovdb_buf_t buf; // = pnanovdb_make_buf(ptr, size / 4);

    // 射线的方向可以 由 观察点 和 UV 坐标计算得出
    pnanovdb_vec3_t world_p = pnanovdb_vec3_t(-110, 0, 0);
    pnanovdb_vec3_t world_d = pnanovdb_vec3_t(1, 0, 0);
    float tmax = 1000;
    float tmin = 0;

//    outFragColor_B8G8R8A8_SRGB = vec4(check_grid_class(0, PNANOVDB_GRID_CLASS_LEVEL_SET), 0);

    if (trace_nanovdb_levelset(buf, world_p, world_d, tmin, tmax) == true) {
        outFragColor_B8G8R8A8_SRGB = vec4(1.0, 0, 0, 0);
    } else {
        // 不相交的时候就忽略当前像素的颜色
        discard;
    }
}
