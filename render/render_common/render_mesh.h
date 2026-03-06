//
// Created by 潘鑫 on 2026/3/6.
//

#ifndef HELLO_MAC_RENDER_MESH_H
#define HELLO_MAC_RENDER_MESH_H
#include <volk.h>
#include "vulkan_buffer.h"

class Model_mesh {
public:
    // 不做
    VKR_buffer_ptr vertices = {};
    VKR_buffer_ptr indices  = {};
    // VkDeviceSize vertices_offset = 0; // 以字节为单位的偏移
    // VkDeviceSize indices_offset  = 0; // 以字节为单位的偏移
    VkIndexType index_type = VK_INDEX_TYPE_UINT16;

    union {
        VkDrawIndexedIndirectCommand indexed_command = {};
        VkDrawIndirectCommand vertex_command;
    };

    // 多的话上面的两个内容是需要更改为 vector 的，可能还需要 material 的指针

    void draw(const VkCommandBuffer &cb, const uint64_t time_line) {
        if (vertices->get_buffer_handle() == VK_NULL_HANDLE)
            return;
        VkDeviceSize temp_offset = 0;
        vkCmdBindVertexBuffers(cb, 0, 1, vertices->get_buffer_handle_ptr(time_line), &temp_offset);
        if (indices->get_buffer_handle() != VK_NULL_HANDLE && indexed_command.indexCount != 0) {
            vkCmdBindIndexBuffer(cb, indices->get_buffer_handle(), 0, index_type);
            vkCmdDrawIndexed(cb, indexed_command.indexCount,
                             indexed_command.instanceCount,
                             indexed_command.firstIndex,
                             indexed_command.vertexOffset,
                             indexed_command.firstInstance);
        } else if (vertex_command.vertexCount != 0) {
            vkCmdDraw(cb, vertex_command.vertexCount,
                      vertex_command.instanceCount,
                      vertex_command.firstVertex,
                      vertex_command.firstInstance);
        }
        // gl_InstanceIndex 只与 instanceCount 和 firstInstance 有关，不会和前一个 VkDrawIndirectCommand 有关的
    }
};


class Model_mesh_vector {
public:
    // 不做
    VKR_buffer_ptr vertices = {};
    VKR_buffer_ptr indices  = {};
    // 其实还是要去分区的，看看那些内容在变化，哪些内容没有变化
    // 为什么2d的内容可以 变化时 覆盖原有内容，而 3d 不行呢 ？

    union draw_command {
        VkDrawIndexedIndirectCommand indexed_command = {};
        VkDrawIndirectCommand vertex_command;
    };

    std::vector<draw_command> draw_commands; // 需要 传输到 GPU ，没有做
    VKR_buffer_ptr draw_commands_buffer = {};
    VkIndexType index_type              = VK_INDEX_TYPE_UINT16;


    void draw(const VkCommandBuffer &cb, const uint64_t time_line) const {
        vkCmdBindVertexBuffers(cb, 0, 1, vertices->get_buffer_handle_ptr(time_line), 0);
        if (indices->get_buffer_handle() != VK_NULL_HANDLE) {
            vkCmdBindIndexBuffer(cb, indices->get_buffer_handle(), 0, index_type);
            vkCmdDrawIndexedIndirect(cb,
                                     // draw_commands 相关内容
                                     draw_commands_buffer->get_buffer_handle(),
                                     0,
                                     draw_commands.size(),
                                     sizeof(draw_command));
        } else {
            vkCmdDrawIndirect(cb,
                              // draw_commands 相关内容
                              draw_commands_buffer->get_buffer_handle(),
                              0,
                              draw_commands.size(),
                              sizeof(draw_command)
                             );
        }
    }
};


#endif //HELLO_MAC_RENDER_MESH_H
