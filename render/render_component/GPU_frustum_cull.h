//
// Created by 潘鑫 on 2026/7/29.
//

#ifndef HELLO_MAC_COMMAND_CALCULATE_H
#define HELLO_MAC_COMMAND_CALCULATE_H
#include "../render_common/frustum.h"
#include "vulkan_buffer.h"

struct GPU_frustum_cull {
    FrustumPlanes frustum_planes;
    uint64_t AABB_boxesAddress;
    uint64_t PrimitivesAddress;
    uint64_t frustum_cullingAddress;
    uint32_t command_size;
    uint32_t hiz_max_lod;
};

#endif //HELLO_MAC_COMMAND_CALCULATE_H
