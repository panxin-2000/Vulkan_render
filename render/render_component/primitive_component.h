//
// Created by 潘鑫 on 2026/3/6.
//

#ifndef HELLO_MAC_RENDER_MESH_H
#define HELLO_MAC_RENDER_MESH_H
#include <volk.h>
#include <absl/container/flat_hash_map.h>

#include "../render_common/Geometry_data.h"
#include "vertex_and_buffer_index.h"
#include "vulkan_buffer.h"


class VKR_Primitive {
public:
    uint32_t indexCount    = 0;
    uint32_t instanceCount = 0;
    uint32_t firstIndex    = 0;
    int32_t vertexOffset   = 0;
    uint32_t firstInstance = 0;
    // uint32_t material_index_ = 0;
};


struct Draw_command {
    union {
        VkDrawIndexedIndirectCommand indexed_command = {};
        VkDrawIndirectCommand vertex_command;
        // 想实现 一个 绘制的命令，代价似乎有点大
        // 怎么大呢？ firstInstance  instanceCount
        // 只有这两个参数
        // 那么 意味 着什么呢？AABB  model pbr_material vertices indices 等等都需要排列 到一个 buffer 上
        // 其实问题不只是 排列到 各自到 buffer 上，而是需要 按照顺序来排列
        //
    };
};


std::vector<VKR_Primitive> create_primitives(entt::entity entity);

std::vector<VKR_Primitive> create_primitives(const Geometry_data &data);


#endif //HELLO_MAC_RENDER_MESH_H
