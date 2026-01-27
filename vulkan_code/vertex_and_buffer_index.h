//
// Created by 潘鑫 on 2026/1/26.
//

#ifndef HOWTOVULKAN_VERTEX_AND_BUFFER_INDEX_H
#define HOWTOVULKAN_VERTEX_AND_BUFFER_INDEX_H
#include <volk.h>
#include "vulkan_device_handle.h"
#include <tiny_obj_loader.h>


std::tuple<VkBuffer, uint32_t, uint32_t> create_mesh_data(VKDevice &handle, VmaAllocation &vBufferAllocation) {
    VkBuffer vBuffer{VK_NULL_HANDLE};

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    auto result = tinyobj::LoadObj(&attrib, &shapes, &materials, nullptr, nullptr, "assets/suzanne.obj");
    // todo : result need check
    const VkDeviceSize indexCount{shapes[0].mesh.indices.size()};
    std::vector<Vertex> vertices{};
    std::vector<uint16_t> indices{};
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
        vertices.push_back(v);
        indices.push_back(indices.size());
    }
    VkDeviceSize vBufSize{sizeof(Vertex) * vertices.size()};
    VkDeviceSize iBufSize{sizeof(uint16_t) * indices.size()};
    VkBufferCreateInfo bufferCI{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = vBufSize + iBufSize,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
    };
    VmaAllocationCreateInfo bufferAllocCI{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    VK_CHECK_RESULT(vmaCreateBuffer(handle.get_allocator(), &bufferCI, &bufferAllocCI, &vBuffer, &vBufferAllocation, nullptr));
    void *bufferPtr{nullptr};
    VK_CHECK_RESULT(vmaMapMemory(handle.get_allocator(), vBufferAllocation, &bufferPtr));
    memcpy(bufferPtr, vertices.data(), vBufSize);
    memcpy(((char *) bufferPtr) + vBufSize, indices.data(), iBufSize);
    vmaUnmapMemory(handle.get_allocator(), vBufferAllocation);
    return {vBuffer, vBufSize, indexCount};
}

#endif //HOWTOVULKAN_VERTEX_AND_BUFFER_INDEX_H
