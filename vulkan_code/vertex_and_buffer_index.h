//
// Created by 潘鑫 on 2026/1/26.
//

#ifndef HOWTOVULKAN_VERTEX_AND_BUFFER_INDEX_H
#define HOWTOVULKAN_VERTEX_AND_BUFFER_INDEX_H
#include <volk.h>
#include "vulkan_device_handle.h"
#include <tiny_obj_loader.h>

#include "shader_common.h"

struct Model_mesh {
    // 不做
    VkBuffer vertices_buffer     = VK_NULL_HANDLE;
    VkDeviceSize vertices_offset = 0; // 以字节为单位的偏移
    VkBuffer indices_buffer      = VK_NULL_HANDLE;
    VkDeviceSize indices_offset  = 0; // 以字节为单位的偏移
    VkIndexType index_type       = VK_INDEX_TYPE_UINT16;

    union {
        VkDrawIndexedIndirectCommand indexed_command = {};
        VkDrawIndirectCommand vertex_command;
    };

    void draw(const VkCommandBuffer &cb) {
        vkCmdBindVertexBuffers(cb, 0, 1, &vertices_buffer, &vertices_offset);
        if (indices_buffer != VK_NULL_HANDLE) {
            vkCmdBindIndexBuffer(cb, indices_buffer, indices_offset, index_type);
            vkCmdDrawIndexed(cb, indexed_command.indexCount,
                             indexed_command.instanceCount,
                             indexed_command.firstIndex,
                             indexed_command.vertexOffset,
                             indexed_command.firstInstance);
        } else {
            vkCmdDraw(cb, vertex_command.vertexCount,
                      vertex_command.instanceCount,
                      vertex_command.firstVertex,
                      vertex_command.firstInstance);
        }
    }
};


bool load_model_to_vector(std::shared_ptr<std::vector<Vertex> > &vertices,
                          std::shared_ptr<std::vector<uint16_t> > &indices) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    auto result = tinyobj::LoadObj(&attrib, &shapes, &materials, nullptr, nullptr, "assets/suzanne.obj");
    // todo : result need check
    if (result == false) {
        return false;
    }
    const VkDeviceSize indexCount{shapes[0].mesh.indices.size()};
    // Load vertex and index data
    for (auto &index: shapes[0].mesh.indices) {
        Vertex v{
            .pos = {
                attrib.vertices[index.vertex_index * 3], -attrib.vertices[index.vertex_index * 3 + 1],
                attrib.vertices[index.vertex_index * 3 + 2]
            },
            .normal = {
                attrib.normals[index.normal_index * 3], -attrib.normals[index.normal_index * 3 + 1],
                attrib.normals[index.normal_index * 3 + 2]
            },
            .uv = {attrib.texcoords[index.texcoord_index * 2], 1.0 - attrib.texcoords[index.texcoord_index * 2 + 1]}
        };
        vertices->push_back(v);
        indices->push_back(indices->size());
    }
}

std::pair<vertex_and_attributes, Indices_type> load_model() {
    vertex_and_attributes vertices{};
    Indices_type indices{};
    // std::shared_ptr<std::vector<Vertex> > &vertices; std::shared_ptr<std::vector<uint16_t> > &indices;
    auto sp_vertices = std::make_shared<std::vector<Vertex> >();
    auto sp_indices  = std::make_shared<std::vector<uint16_t> >();
    load_model_to_vector(sp_vertices, sp_indices);
    vertices.shared_ptr_of_vertices_ = sp_vertices;
    vertices.data                    = sp_vertices->data();
    vertices.size                    = sp_vertices->size() * sizeof(Vertex);
    return {vertices, indices};
}


Model_mesh create_mesh_data(VKDevice &handle, vertex_and_attributes vertices, Indices_type indices_,
                            VmaAllocation &vBufferAllocation) {
    VkBuffer vBuffer{VK_NULL_HANDLE};
    Model_mesh mesh{};
    // 到这里应该是结束了一部分内容了吧
    VkDeviceSize vBufSize{vertices.size};
    VkDeviceSize iBufSize{sizeof(uint16_t) * indices_->size()};
    VkBufferCreateInfo bufferCI{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = vBufSize + iBufSize,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
    };
    VmaAllocationCreateInfo bufferAllocCI{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    VK_CHECK_RESULT(
                    vmaCreateBuffer(handle.get_allocator(), &bufferCI, &bufferAllocCI, &vBuffer, &vBufferAllocation,
                        nullptr));
    void *bufferPtr{nullptr};
    VK_CHECK_RESULT(vmaMapMemory(handle.get_allocator(), vBufferAllocation, &bufferPtr));
    memcpy(bufferPtr, vertices.data, vBufSize);
    memcpy(((char *) bufferPtr) + vBufSize, indices_->data(), iBufSize);
    vmaUnmapMemory(handle.get_allocator(), vBufferAllocation);

    mesh.vertices_buffer = vBuffer;
    mesh.indices_buffer  = vBuffer;
    // mesh.indices_offset = vBufSize;
    mesh.indexed_command.indexCount    = indices_->size(); // 是可以这么替换的
    mesh.indexed_command.firstIndex    = vBufSize / 2;     // 索引缓冲区的起始偏移（以索引为单位）确实是可以通过计算偏移的
    mesh.indexed_command.vertexOffset  = 0;
    mesh.indexed_command.instanceCount = 3;
    mesh.indexed_command.firstInstance = 0;


    return mesh;
}

#endif //HOWTOVULKAN_VERTEX_AND_BUFFER_INDEX_H
