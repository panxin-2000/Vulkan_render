//
// Created by 潘鑫 on 2026/1/26.
//

#ifndef HOWTOVULKAN_VERTEX_AND_BUFFER_INDEX_H
#define HOWTOVULKAN_VERTEX_AND_BUFFER_INDEX_H
#include <volk.h>
#include "vulkan_device_handle.h"
#include <tiny_obj_loader.h>

#include "shader_common.h"
#include "vulkan_buffer.h"


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

std::pair<VkBuffer, VmaAllocation> create_vma_buffer(const VKDevice &handle, VkDeviceSize size,
                                                     VkBufferUsageFlags usage, VmaAllocationCreateFlags flags) {
    VkBuffer vBuffer{VK_NULL_HANDLE};
    VmaAllocation vBufferAllocation{VK_NULL_HANDLE};
    // 到这里应该是结束了一部分内容了吧
    VkDeviceSize vBufSize = size;
    VkBufferCreateInfo BufferCreateInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size  = vBufSize,
        .usage = usage
    };
    VmaAllocationCreateInfo AllocationCreateInfo{
        .flags = flags,
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    VmaAllocationInfo allocInfo = {};
    VK_CHECK_RESULT(vmaCreateBuffer(handle.get_allocator(),
                        &BufferCreateInfo, &AllocationCreateInfo,
                        &vBuffer, &vBufferAllocation,
                        &allocInfo));
    return {vBuffer, vBufferAllocation};
}


inline bool check_host_visible_bit(const VKDevice &handle, const VmaAllocation vBufferAllocation) {
    VmaAllocationInfo info;
    vmaGetAllocationInfo(handle.get_allocator(), vBufferAllocation, &info);
    VkMemoryPropertyFlags props;
    vmaGetMemoryTypeProperties(handle.get_allocator(), info.memoryType, &props);
    const bool isVisible = props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    return isVisible;
}

inline bool check_need_flush_bit(const VKDevice &handle, const VmaAllocation vBufferAllocation) {
    VmaAllocationInfo info;
    vmaGetAllocationInfo(handle.get_allocator(), vBufferAllocation, &info);
    VkMemoryPropertyFlags props;
    vmaGetMemoryTypeProperties(handle.get_allocator(), info.memoryType, &props);
    const bool need_flush = props & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    return need_flush;
}

inline bool copy_mem_from_cpu_to_gpu(const VKDevice &handle,
                                     const std::pair<VkBuffer, VmaAllocation> &buffer_handle,
                                     const std::function<void(void *)> &mem_copy_callback) {
    if (check_host_visible_bit(handle, buffer_handle.second) == true) {
        void *bufferPtr{nullptr};
        VK_CHECK_RESULT(vmaMapMemory(handle.get_allocator(), buffer_handle.second, &bufferPtr));
        // 也可以通过下面两行获取 map 的地址 ，取其中的 pMappedData
        VmaAllocationInfo allocInfo_for_map;
        vmaGetAllocationInfo(handle.get_allocator(), buffer_handle.second, &allocInfo_for_map);
        if (mem_copy_callback != nullptr) {
            mem_copy_callback(allocInfo_for_map.pMappedData);
        }
        if (check_need_flush_bit(handle, buffer_handle.second) == false) {
            vmaFlushAllocation(handle.get_allocator(), buffer_handle.second, 0, allocInfo_for_map.size);
        }
        vmaUnmapMemory(handle.get_allocator(), buffer_handle.second);
        return true;
    } else {
        return false;
    }
}


// 最差结果 总是 CPU 可见, GPU 通过 PCIE 读取数据
inline std::pair<VkBuffer, VmaAllocation> create_staging_buffer(const VKDevice &handle, VkDeviceSize size) {
    return create_vma_buffer(handle, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
}

/**
 *  总会创建成功，除非内存不够，返回 都为 VK_NULL_HANDLE
 * @param handle
 * @param size
 * @param mem_copy_callback
 * @return
 */
std::pair<VkBuffer, VmaAllocation> create_vertex_index_buffer(const VKDevice &handle, VkDeviceSize size,
                                                              std::function<void(void *)> mem_copy_callback) {
    auto [vBuffer,vBufferAllocation] =
            create_vma_buffer(handle, size,
                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                              VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                              VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
                              VK_BUFFER_USAGE_TRANSFER_DST_BIT); // 最差结果 纯显存（DEVICE_LOCAL）
    if (check_host_visible_bit(handle, vBufferAllocation) == false) {
        LOG_INFO(g_log(), "can find a cpu write memory, only get GPU memory", size);
        auto [staging_buffer,staging_allocation] = create_staging_buffer(handle, size);
        if (check_host_visible_bit(handle, staging_allocation) == false) {
            LOG_INFO(g_log(), "can find a cpu write memory, allocate size {}", size);
        } else {
            copy_mem_from_cpu_to_gpu(handle, {staging_buffer, staging_allocation}, mem_copy_callback);
            copy_vk_buffer(handle, staging_buffer, vBuffer, size);
        }
        vmaDestroyBuffer(handle.get_allocator(), staging_buffer, staging_allocation);
    } else {
        copy_mem_from_cpu_to_gpu(handle, {vBuffer, vBufferAllocation}, mem_copy_callback);
    }
    return {vBuffer, vBufferAllocation};
}


inline Model_mesh create_mesh_data(const VKDevice &handle, const vertex_and_attributes &vertices,
                                   const Indices_type &indices_) {
    VkDeviceSize vBufSize{vertices.size};
    VkDeviceSize iBufSize{sizeof(uint16_t) * indices_->size()};

    // 具体的复制函数
    auto mem_copy_function = [vertices,vBufSize,indices_,iBufSize](void *dst) {
        memcpy(dst, vertices.data, vBufSize);
        memcpy(static_cast<char *>(dst) + vBufSize, indices_->data(), iBufSize);
    };

    auto [vBuffer,vBufferAllocation] =
            create_vertex_index_buffer(handle, vBufSize + iBufSize, mem_copy_function);


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
