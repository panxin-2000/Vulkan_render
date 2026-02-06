//
// Created by 潘鑫 on 2026/1/26.
//

#ifndef HOWTOVULKAN_VERTEX_AND_BUFFER_INDEX_H
#define HOWTOVULKAN_VERTEX_AND_BUFFER_INDEX_H
#include <volk.h>
#include "vulkan_device_handle.h"
#include <tiny_obj_loader.h>

#include "shader_common.h"


inline bool load_model_to_vector(const std::string &path, std::shared_ptr<std::vector<Vertex> > &vertices,
                                 std::shared_ptr<std::vector<uint16_t> > &indices) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    auto result = tinyobj::LoadObj(&attrib, &shapes, &materials, nullptr, nullptr, path.c_str());
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

inline std::pair<vertex_and_attributes, Indices_type> load_model(const std::string &path) {
    vertex_and_attributes vertices{};
    // std::shared_ptr<std::vector<Vertex> > &vertices; std::shared_ptr<std::vector<uint16_t> > &indices;
    auto sp_vertices = std::make_shared<std::vector<Vertex> >();
    auto sp_indices  = std::make_shared<std::vector<uint16_t> >();
    load_model_to_vector(path, sp_vertices, sp_indices);
    vertices.shared_ptr_of_vertices_ = sp_vertices;
    vertices.data                    = sp_vertices->data();
    vertices.size                    = sp_vertices->size() * sizeof(Vertex);
    return {vertices, sp_indices};
}


inline Model_mesh create_mesh_data(const VKDevice &handle, const vertex_and_attributes &vertices,
                                   const Indices_type &indices_) {
    VkBuffer vBuffer{VK_NULL_HANDLE};
    VmaAllocation vBufferAllocation{VK_NULL_HANDLE};
    // 到这里应该是结束了一部分内容了吧
    VkDeviceSize vBufSize{vertices.size};
    VkDeviceSize iBufSize{sizeof(uint16_t) * indices_->size()};
    VkBufferCreateInfo bufferCI{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size  = vBufSize + iBufSize,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
    };
    VmaAllocationCreateInfo bufferAllocCI{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        // 之前是有 VMA_ALLOCATION_CREATE_MAPPED_BIT 的标记的
        // 这个标记 表示已经 map ,但是 调用 vmaUnmapMemory 会 assert 失败
        // 这个标记 一直伴随着 VkBuffer 和 VmaAllocation ，直到销毁
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    VmaAllocationInfo allocInfo = {};
    VK_CHECK_RESULT(
                    vmaCreateBuffer(handle.get_allocator(), &bufferCI, &bufferAllocCI, &vBuffer, &vBufferAllocation,
                        &allocInfo));
    // 2. 检查 VMA 到底把内存分到了哪里
    VkMemoryPropertyFlags memFlags;
    vmaGetMemoryTypeProperties(handle.get_allocator(), allocInfo.memoryType, &memFlags);
    if (memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        void *bufferPtr{nullptr};
        VK_CHECK_RESULT(vmaMapMemory(handle.get_allocator(), vBufferAllocation, &bufferPtr));
        // 也可以通过下面两行获取 map 的地址 ，取其中的 pMappedData
        VmaAllocationInfo allocInfo_for_map;
        vmaGetAllocationInfo(handle.get_allocator(), vBufferAllocation, &allocInfo_for_map);
        memcpy(allocInfo_for_map.pMappedData, vertices.data, vBufSize);
        memcpy(((char *) allocInfo_for_map.pMappedData) + vBufSize, indices_->data(), iBufSize);
        vmaUnmapMemory(handle.get_allocator(), vBufferAllocation);
    } else {
        // 需要手动搬运
    }

    Model_mesh mesh{};
    mesh.vertices_buffer   = vBuffer;
    mesh.vBufferAllocation = vBufferAllocation;
    mesh.indices_buffer    = vBuffer;
    // mesh.indices_offset = vBufSize;
    mesh.indexed_command.indexCount    = indices_->size(); // 是可以这么替换的
    mesh.indexed_command.firstIndex    = vBufSize / 2;     // 索引缓冲区的起始偏移（以索引为单位）确实是可以通过计算偏移的
    mesh.indexed_command.vertexOffset  = 0;
    mesh.indexed_command.instanceCount = 1;
    mesh.indexed_command.firstInstance = 0;


    return mesh;
}


inline void create_mesh(const VKDevice &handle, logic_render_data *data,
                        std::map<logic_render_data *, buffer_and_share> &map) {
    if (data != nullptr) {
        auto it = map.find(data);
        if (it != map.end()) {
            it->second.shared_number++;
        } else {
            if (data->mesh_path_.empty() == false) {
                auto [vertices, indices] = load_model(data->mesh_path_);
                const auto mesh          = create_mesh_data(handle, vertices, indices);
                map.insert({data, {mesh, 1}});
            } else {
                for (const auto &temp: data->vertex_and_attributes_) {
                    // create_vertex_buffer(temp.shared_ptr_of_vertices_, temp.size, temp.data, &vertices_map_);
                    auto mesh = create_mesh_data(handle, temp, data->indices_);
                    map.insert({data, {mesh, 1}});
                }
            }
        }
    }
}


inline Model_mesh *find_mesh(logic_render_data *data,
                             std::map<logic_render_data *, buffer_and_share> &map) {
    if (data != nullptr) {
        auto it = map.find(data);
        if (it != map.end()) {
            return &it->second.mesh;
        } else {
            return nullptr;
        }
    }
}
#endif //HOWTOVULKAN_VERTEX_AND_BUFFER_INDEX_H
