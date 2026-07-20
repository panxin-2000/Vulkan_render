#define PNANOVDB_GLSL
#define PNANOVDB_ADDRESS_32
#include "PNanoVDB.h"


PNANOVDB_FORCE_INLINE float pnanovdb_hdda_get_distance(
    pnanovdb_grid_type_t grid_type, // pnanovdb_grid_get_grid_type 获取
    pnanovdb_buf_t buf,             //
    PNANOVDB_INOUT(pnanovdb_readaccessor_t) acc,
    PNANOVDB_IN(pnanovdb_vec3_t) origin_position, // 已经是第一个碰撞的位置了
    float tmin,
    PNANOVDB_IN(pnanovdb_vec3_t) direction,
    float tmax) {
    // 拿到第一个相交的 世界坐标
    pnanovdb_vec3_t last_position = pnanovdb_hdda_ray_start(origin_position, tmin, direction);
    // 转换坐标
    pnanovdb_coord_t ijk = pnanovdb_hdda_pos_to_ijk(PNANOVDB_REF(last_position));

    pnanovdb_int32_t dim = pnanovdb_uint32_as_int32(
                                                    pnanovdb_readaccessor_get_dim(PNANOVDB_GRID_TYPE_FLOAT,
                                                             buf,
                                                             acc,
                                                             PNANOVDB_REF(ijk)));
    pnanovdb_hdda_t hdda;
    float total_distance = 0.0f;

    // 不是重新创建了一个，而是首次创建了一个  // 这里的dim 才是一个正确的创建方式
    pnanovdb_hdda_init(PNANOVDB_REF(hdda), origin_position, tmin, direction, tmax, dim);
    // 开始步进  结果会存储 在 hdda 中
    while (pnanovdb_hdda_step(PNANOVDB_REF(hdda))) {
        pnanovdb_vec3_t new_position = pnanovdb_hdda_ray_start(origin_position, hdda.tmin + 1.0001f, direction);
        ijk                          = pnanovdb_hdda_pos_to_ijk(PNANOVDB_REF(new_position));
        dim                          = pnanovdb_uint32_as_int32(
                                       pnanovdb_readaccessor_get_dim(PNANOVDB_GRID_TYPE_FLOAT,
                                                                     buf,
                                                                     acc,
                                                                     PNANOVDB_REF(ijk)));
        ijk                        = hdda.voxel;
        pnanovdb_address_t address = pnanovdb_readaccessor_get_value_address(PNANOVDB_GRID_TYPE_FLOAT,
                                                                             buf,
                                                                             acc,
                                                                             PNANOVDB_REF(ijk));
        pnanovdb_hdda_update(PNANOVDB_REF(hdda), origin_position, direction, dim);
        if (pnanovdb_read_float(buf, address) < 0.f) {
            total_distance = total_distance + length(new_position - last_position);
            last_position  = new_position;
            continue;
        }
        last_position = new_position;
        if (dim > 1 && !pnanovdb_readaccessor_is_active(grid_type, buf, acc, PNANOVDB_REF(hdda.voxel))) {
            break;
        }
    }
    return total_distance;
}
