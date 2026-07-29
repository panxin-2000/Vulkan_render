//
// Created by 潘鑫 on 2026/3/6.
//

#ifndef HELLO_MAC_RENDER_MESH_H
#define HELLO_MAC_RENDER_MESH_H
#include <volk.h>
#include "vulkan_buffer.h"

class Mesh_data {
public:
    VKR_buffer_ptr vertices      = {};
    VKR_buffer_ptr indices       = {};
    VkDeviceSize vertices_offset = 0; // 以字节为单位的偏移
    VkDeviceSize indices_offset  = 0; // 以字节为单位的偏移
    VkIndexType index_type       = VK_INDEX_TYPE_UINT16;
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

class VKR_Primitive {
public:
    // 不做
    Draw_command draw_command;
    int material_index_ = 0;


    // 多的话上面的两个内容是需要更改为 vector 的，可能还需要 material 的指针


    // gl_InstanceIndex 只与 instanceCount 和 firstInstance 有关，不会和前一个 VkDrawIndirectCommand 有关的
};



//
class Draw_commands {
public:
    // 不做
    VkDeviceSize vertices_offset = 0; // 以字节为单位的偏移
    VkDeviceSize indices_offset  = 0; // 以字节为单位的偏移

    // 其实还是要去分区的，看看那些内容在变化，哪些内容没有变化
    // 为什么2d的内容可以 变化时 覆盖原有内容，而 3d 不行呢 ？
    std::vector<Draw_command> draw_commands; // 需要 传输到 GPU ，没有做
    VKR_buffer_ptr draw_commands_buffer = {};
    VkIndexType index_type              = VK_INDEX_TYPE_UINT16;


    void draw(const VkCommandBuffer &cb, const Mesh_data &mesh_data, const uint64_t time_line) const {
        vkCmdBindVertexBuffers(cb, 0, 1, mesh_data.vertices->get_buffer_handle_ptr(time_line), &vertices_offset);
        if (mesh_data.indices->get_buffer_handle() != VK_NULL_HANDLE) {
            vkCmdBindIndexBuffer(cb, mesh_data.indices->get_buffer_handle(), indices_offset, index_type);
            vkCmdDrawIndexedIndirect(cb,
                                     // draw_commands 相关内容
                                     draw_commands_buffer->get_buffer_handle(),
                                     0,
                                     draw_commands.size(),
                                     sizeof(Draw_command));
        } else {
            vkCmdDrawIndirect(cb,
                              // draw_commands 相关内容
                              draw_commands_buffer->get_buffer_handle(),
                              0,
                              draw_commands.size(),
                              sizeof(Draw_command)
                             );
        }
    }
};


#endif //HELLO_MAC_RENDER_MESH_H
