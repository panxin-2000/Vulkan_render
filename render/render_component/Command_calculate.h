//
// Created by 潘鑫 on 2026/7/29.
//

#ifndef HELLO_MAC_COMMAND_CALCULATE_H
#define HELLO_MAC_COMMAND_CALCULATE_H
#include "frustum.h"


struct Command_calculate {
    FrustumPlanes frustum_planes;
    uint64_t AABB_boxesAddress;
    uint64_t IndirectCommandsAddress;
    uint32_t command_size;
    VKR_buffer_ptr command_buffer;
    VKR_buffer_ptr AABB_boxes_buffer;
};

#endif //HELLO_MAC_COMMAND_CALCULATE_H
