#define PNANOVDB_GLSL
#define PNANOVDB_ADDRESS_32
#include "PNanoVDB.h"
#define pnanovdb_uint32_t uint
#define pnanovdb_int32_t int
#define pnanovdb_bool_t bool
#define PNANOVDB_FALSE false
#define PNANOVDB_TRUE true
#define pnanovdb_uint64_t uvec2
#define pnanovdb_int64_t ivec2
#define pnanovdb_coord_t ivec3
#define pnanovdb_vec3_t vec3
#define PNANOVDB_STRUCT_TYPEDEF(X)
#define PNANOVDB_STATIC_CONST const
#define PNANOVDB_INOUT(X) inout X
#define PNANOVDB_IN(X) X
#define PNANOVDB_DEREF(X) X
#define PNANOVDB_REF(X) X
#define PNANOVDB_FORCE_INLINE

#include "VdbCommon.glsl"


/**
*
*  grid_type  pnanovdb_grid_get_grid_type 获取
*  origin_position 是索引空间的位置
*  tmin 一遍情况下是 光线的起点
*  direction 表示光线的方向, 索引空间
*  tmax 光线最远能走多远
**/
PNANOVDB_FORCE_INLINE float vdb_get_out_distance(VdbSampler vdb_sampler,
                                                 PNANOVDB_IN(pnanovdb_vec3_t) origin_position, float tmin,
                                                 PNANOVDB_IN(pnanovdb_vec3_t) direction, float tmax) {
    // 拿到光线的 能够到大的第一个位置 ,大部分情况下 就是 origin_position 没有任何改变
    pnanovdb_vec3_t position = pnanovdb_hdda_ray_start(origin_position, tmin, direction);
    // 转换坐标  pnanovdb_coord_t 是 ivec3 是 int 类型的 三个值 ,用于 索引具体的体素 , 只是改变了 值 的类型
    pnanovdb_coord_t ijk = pnanovdb_hdda_pos_to_ijk(PNANOVDB_REF(position));
    pnanovdb_int32_t dim = pnanovdb_uint32_as_int32(pnanovdb_readaccessor_get_dim(PNANOVDB_GRID_TYPE_FLOAT,
                                                                                  vdb_sampler.GridBuffer,
                                                                                  vdb_sampler.Accessor,
                                                                                  PNANOVDB_REF(ijk)));
    pnanovdb_hdda_t hdda;
    float total_distance = 0.0f;

    // 不是重新创建了一个，而是首次创建了一个
    // 这里的问题应该是 dim 是可以 pnanovdb_hdda_init 函数中获取的,但是为什么要传递一遍呢?
    pnanovdb_hdda_init(PNANOVDB_REF(hdda), origin_position, tmin, direction, tmax, dim);
    // 开始步进  结果会存储 在 hdda 中
    // pnanovdb_hdda_step 只会步进 ,然后 最大步进到 tmax 就停止了
    // 如果我需要计算边界的话,那么就 需要手动计算 到 box 到距离了
    while (pnanovdb_hdda_step(PNANOVDB_REF(hdda))) {
        pnanovdb_vec3_t light_reach_position = pnanovdb_hdda_ray_start(origin_position, hdda.tmin, direction);
        ijk = pnanovdb_hdda_pos_to_ijk(PNANOVDB_REF(light_reach_position));
        dim = pnanovdb_uint32_as_int32(pnanovdb_readaccessor_get_dim(PNANOVDB_GRID_TYPE_FLOAT,
                                                                     vdb_sampler.GridBuffer,
                                                                     vdb_sampler.Accessor,
                                                                     PNANOVDB_REF(ijk)));
        ijk = hdda.voxel;
        pnanovdb_address_t address = pnanovdb_readaccessor_get_value_address(PNANOVDB_GRID_TYPE_FLOAT,
                                                                             vdb_sampler.GridBuffer,
                                                                             vdb_sampler.Accessor,
                                                                             PNANOVDB_REF(ijk));
        pnanovdb_hdda_update(PNANOVDB_REF(hdda), origin_position, direction, dim);

        if (pnanovdb_read_float(vdb_sampler.GridBuffer, address) < 0.f) {
            total_distance = total_distance + length(light_reach_position - position);
            // 其实这里稍微不太准 应该 是 单个网格的 维度 来确定距离的
        }
        position = light_reach_position;
    }
    return total_distance;
}



/**
*
*  grid_type  pnanovdb_grid_get_grid_type 获取
*  origin_position 是索引空间的位置
*  tmin 一遍情况下是 光线的起点
*  direction 表示光线的方向, 索引空间
*  tmax 光线最远能走多远
**/
PNANOVDB_FORCE_INLINE float vdb_get_ray_density(VdbSampler vdb_sampler,
                                                PNANOVDB_IN(pnanovdb_vec3_t) origin_position, float tmin,
                                                PNANOVDB_IN(pnanovdb_vec3_t) direction, float tmax) {
    // 拿到光线的 能够到大的第一个位置 ,大部分情况下 就是 origin_position 没有任何改变
    pnanovdb_vec3_t position = pnanovdb_hdda_ray_start(origin_position, tmin, direction);
    // 转换坐标  pnanovdb_coord_t 是 ivec3 是 int 类型的 三个值 ,用于 索引具体的体素 , 只是改变了 值 的类型
    pnanovdb_coord_t ijk = pnanovdb_hdda_pos_to_ijk(PNANOVDB_REF(position));
    pnanovdb_int32_t dim = pnanovdb_uint32_as_int32(pnanovdb_readaccessor_get_dim(PNANOVDB_GRID_TYPE_FLOAT,
                                                                                  vdb_sampler.GridBuffer,
                                                                                  vdb_sampler.Accessor,
                                                                                  PNANOVDB_REF(ijk)));
    pnanovdb_hdda_t hdda;
    float total_density = 0.0f;

    // 不是重新创建了一个，而是首次创建了一个
    // 这里的问题应该是 dim 是可以 pnanovdb_hdda_init 函数中获取的,但是为什么要传递一遍呢?
    pnanovdb_hdda_init(PNANOVDB_REF(hdda), origin_position, tmin, direction, tmax, dim);
    // 开始步进  结果会存储 在 hdda 中
    // pnanovdb_hdda_step 只会步进 ,然后 最大步进到 tmax 就停止了
    // 如果我需要计算边界的话,那么就 需要手动计算 到 box 到距离了
    while (pnanovdb_hdda_step(PNANOVDB_REF(hdda))) {
        pnanovdb_vec3_t light_reach_position = pnanovdb_hdda_ray_start(origin_position, hdda.tmin, direction);
        ijk = pnanovdb_hdda_pos_to_ijk(PNANOVDB_REF(light_reach_position));
        dim = pnanovdb_uint32_as_int32(pnanovdb_readaccessor_get_dim(PNANOVDB_GRID_TYPE_FLOAT,
                                                                     vdb_sampler.GridBuffer,
                                                                     vdb_sampler.Accessor,
                                                                     PNANOVDB_REF(ijk)));
        ijk = hdda.voxel;
        pnanovdb_address_t address = pnanovdb_readaccessor_get_value_address(PNANOVDB_GRID_TYPE_FLOAT,
                                                                             vdb_sampler.GridBuffer,
                                                                             vdb_sampler.Accessor,
                                                                             PNANOVDB_REF(ijk));
        pnanovdb_hdda_update(PNANOVDB_REF(hdda), origin_position, direction, dim);

        float density = pnanovdb_read_float(vdb_sampler.GridBuffer, address);
        total_density = total_density + length(light_reach_position - position) * density;
        // 其实这里稍微不太准 应该 是 单个网格的 维度 来确定距离的
        position = light_reach_position;
    }
    return total_density;
}





/**
*  从体积内,某一个点,到达 体积的 外部,应该怎么 算,首先 假设 是存在太阳光的,也只存在 太阳光
*  穿过一点 密度 ,光线就少一点
*  grid_type  pnanovdb_grid_get_grid_type 获取
*  origin_position 是索引空间的位置
*  tmin 一遍情况下是 光线的起点
*  direction 表示光线的方向, 索引空间
*  tmax 光线最远能走多远
**/
PNANOVDB_FORCE_INLINE float vdb_get_out_density(pnanovdb_grid_type_t grid_type, float sigma_a,
                                                pnanovdb_buf_t density_buf, inout pnanovdb_readaccessor_t density_acc,
                                                pnanovdb_buf_t sdf_buf, inout pnanovdb_readaccessor_t sdf_acc,
                                                PNANOVDB_IN(pnanovdb_vec3_t) origin_position, float tmin,
                                                PNANOVDB_IN(pnanovdb_vec3_t) direction, float tmax) {
    // 拿到光线的 能够到大的第一个位置 ,大部分情况下 就是 origin_position 没有任何改变
    pnanovdb_vec3_t position = pnanovdb_hdda_ray_start(origin_position, tmin, direction);
    // 转换坐标  pnanovdb_coord_t 是 ivec3 是 int 类型的 三个值 ,用于 索引具体的体素 , 只是改变了 值 的类型
    pnanovdb_coord_t ijk = pnanovdb_hdda_pos_to_ijk(PNANOVDB_REF(position));
    pnanovdb_int32_t dim = pnanovdb_uint32_as_int32(pnanovdb_readaccessor_get_dim(PNANOVDB_GRID_TYPE_FLOAT,
                                                                                  sdf_buf,
                                                                                  sdf_acc,
                                                                                  PNANOVDB_REF(ijk)));
    pnanovdb_hdda_t hdda;
    float total_density = 0.0f;
    float transmission = 1.0f;


    // 不是重新创建了一个，而是首次创建了一个
    // 这里的问题应该是 dim 是可以 pnanovdb_hdda_init 函数中获取的,但是为什么要传递一遍呢?
    pnanovdb_hdda_init(PNANOVDB_REF(hdda), origin_position, tmin, direction, tmax, dim);
    // 开始步进  结果会存储 在 hdda 中
    // pnanovdb_hdda_step 只会步进 ,然后 最大步进到 tmax 就停止了
    // 如果我需要计算边界的话,那么就 需要手动计算 到 box 到距离了
    while (pnanovdb_hdda_step(PNANOVDB_REF(hdda))) {
        pnanovdb_vec3_t light_reach_position = pnanovdb_hdda_ray_start(origin_position, hdda.tmin, direction);
        ijk = pnanovdb_hdda_pos_to_ijk(PNANOVDB_REF(light_reach_position));
        dim = pnanovdb_uint32_as_int32(pnanovdb_readaccessor_get_dim(PNANOVDB_GRID_TYPE_FLOAT,
                                                                     sdf_buf,
                                                                     sdf_acc,
                                                                     PNANOVDB_REF(ijk)));
        ijk = hdda.voxel;
        pnanovdb_address_t SDF_address = pnanovdb_readaccessor_get_value_address(PNANOVDB_GRID_TYPE_FLOAT,
                                                                                 sdf_buf,
                                                                                 sdf_acc,
                                                                                 PNANOVDB_REF(ijk));

        pnanovdb_address_t density_address = pnanovdb_readaccessor_get_value_address(PNANOVDB_GRID_TYPE_FLOAT,
                                                                                     density_buf,
                                                                                     density_acc,
                                                                                     PNANOVDB_REF(ijk));

        if (pnanovdb_read_float(sdf_buf, SDF_address) < 0.f) {
            //            float density = pnanovdb_read_float(density_buf, density_acc);
            //            float transmission = exp(-distance * sigma_a);

            //            total_density = total_density + length(light_reach_position - position) * density;
        }
        pnanovdb_hdda_update(PNANOVDB_REF(hdda), origin_position, direction, dim);
        position = light_reach_position;
    }
    return total_density;
}
