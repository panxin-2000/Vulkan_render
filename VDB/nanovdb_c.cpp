//
// Created by 潘鑫 on 2026/3/30.
//


#define PNANOVDB_C
#define PNANOVDB_HDDA
#include <stdbool.h>

#include "../render/shader/PNanoVDB.h"


struct VdbSampler {
    pnanovdb_grid_handle_t Grid;
    pnanovdb_buf_t GridBuffer;
    pnanovdb_readaccessor_t Accessor;
    pnanovdb_uint32_t GridType;
    pnanovdb_root_handle_t Root;
};

struct VdbSampler InitVdbSampler(pnanovdb_buf_t buf) {
    struct VdbSampler Sampler;
    Sampler.GridBuffer = buf;

    pnanovdb_address_t address;
    address.byte_offset  = 0;
    Sampler.Grid.address = address;

    pnanovdb_buf_t root_buf     = buf; //
    pnanovdb_tree_handle_t tree = pnanovdb_grid_get_tree(Sampler.GridBuffer, Sampler.Grid);
    Sampler.Root                = pnanovdb_tree_get_root(root_buf, tree);

    pnanovdb_readaccessor_init(&Sampler.Accessor, Sampler.Root);

    Sampler.GridType = pnanovdb_grid_get_grid_type(Sampler.GridBuffer, Sampler.Grid);

    return Sampler;
}

/**
 * 完整函数：在 NanoVDB Grid 中检测射线与等值面（Level Set）的交点
 * @param buf_ptr     NanoVDB 数据的原始内存指针
 * @param grid_offset Grid 在 buffer 中的偏移量（通常为 0）
 * @param world_p     世界空间下的射线起点
 * @param world_d     世界空间下的射线方向（需归一化）
 * @param t_max       射线的最大传播距离
 * @return            如果撞击到等值面返回 true
 */
bool trace_nanovdb_levelset(pnanovdb_buf_t nanovdb_buffer,
                            const pnanovdb_vec3_t world_p,
                            const pnanovdb_vec3_t world_d,
                            float tmin,
                            float tmax) {
    pnanovdb_grid_handle_t Grid;
    pnanovdb_readaccessor_t Accessor;
    pnanovdb_root_handle_t Root;

    pnanovdb_address_t address;
    address.byte_offset = 0;
    Grid.address        = address;

    pnanovdb_tree_handle_t tree = pnanovdb_grid_get_tree(nanovdb_buffer, Grid);
    Root                        = pnanovdb_tree_get_root(nanovdb_buffer, tree);
    pnanovdb_readaccessor_init(&Accessor, Root);
    pnanovdb_uint32_t grid_type = pnanovdb_grid_get_grid_type(nanovdb_buffer, Grid);

    // 只要你拿到了其中一个网格的地址，调用该函数都能得到整个缓冲区包含的网格总数
    auto grid_count = pnanovdb_grid_get_grid_count(nanovdb_buffer, Grid);
    if (grid_count > 1) {
        // 拿到第二个的
        auto next_size = pnanovdb_grid_get_grid_size(nanovdb_buffer, Grid);
        pnanovdb_grid_handle_t Grid_2;
        pnanovdb_address_t address_grid_2;
        address_grid_2.byte_offset = 0;
        Grid_2.address             = address_grid_2;
    }

    // 1. 初始化 Buffer 和 Grid 地址
    // 注意：size_in_words 填入实际大小，或者如果是指针访问模式，填入一个足够大的占位值
    // 这里是创建一个 pnanovdb_buf_t 的方式， 给出地址和最大的大小，在需要检查边界时才最使用最大的大小
    pnanovdb_buf_t buf = pnanovdb_make_buf(nanovdb_buffer.data, 0xFFFFFFFF);


    // 3. 坐标转换：将世界空间射线转到索引空间
    // HDDA 必须在索引空间（Index Space）运行
    pnanovdb_vec3_t index_p = pnanovdb_grid_world_to_indexf(buf, Grid, &world_p);
    pnanovdb_vec3_t index_d = pnanovdb_grid_world_to_index_dirf(buf, Grid, &world_d);

    // 5. 准备输出参数
    pnanovdb_vec3_t hit_ijk; // 撞击点所在的体素索引坐标
    float hit_value;         // 撞击点处的值（通常接近 0）
    float t_hit = 0.0f;      // 输出：撞击时的 t 值（相对于 index_p）

    // 6. 执行 HDDA Zero Crossing 调用
    // 该函数会沿着射线步进，寻找符号变化（正负交替）的点

    float v     = 0;
    bool is_hit = pnanovdb_hdda_zero_crossing(grid_type,
                                              buf,       // Buffer 对象
                                              &Accessor, // 访问器指针
                                              &index_p,  //  origin
                                              tmin,
                                              &index_d, //  direction
                                              tmax,
                                              &t_hit, //
                                              &v
                                             );

    if (is_hit) {
        // 如果需要世界空间下的交点：
        // world_hit = world_p + world_d * (t_hit / length(index_d))
        // 或者直接调用 pnanovdb_grid_index_to_worldf(buf, grid_addr, hit_ijk)
        return true;
    }

    return false;
}

void test(void *ptr, uint64_t size) {
    // handle.bufferSize(): 返回该句柄管理的整个内存缓冲区的字节数
    // handle.gridSize(n): 返回缓冲区中第 n 个特定网格的大小
    // 3. 将 C++ 指针和大小转换为 PNanoVDB 兼容的 Buffer
    // 在 PNanoVDB 中，buffer 通常以 uint32_t (Word) 为单位
    pnanovdb_buf_t buf = pnanovdb_make_buf(static_cast<uint32_t *>(ptr),
                                           static_cast<uint32_t>(size / 4));
    const pnanovdb_vec3_t world_p{-110, 0, 0};
    const pnanovdb_vec3_t world_d{1, 0, 0};
    float tmax = 1000;
    float tmin = 0;
    trace_nanovdb_levelset(buf, world_p, world_d, tmin, tmax);
}
