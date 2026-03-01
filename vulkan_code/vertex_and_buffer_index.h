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


// 最差结果 总是 CPU 可见, GPU 通过 PCIE 读取数据
inline VKR_buffer_ptr create_staging_buffer(const VK_handle &handle, VkDeviceSize size) {
    return create_vma_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
}

inline VKR_buffer_ptr create_vertex_index_buffer(const VK_handle &handle, const VkDeviceSize size) {
    return create_vma_buffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                             VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                             VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT); // 最差结果 纯显存（DEVICE_LOCAL）
}

/**
 *  总会创建成功，除非内存不够，返回 都为 VK_NULL_HANDLE
 * @param handle
 * @param size
 * @param mem_copy_callback
 * @return
 */
inline VKR_buffer_ptr create_vertex_index_buffer(const VK_handle &handle, VkDeviceSize size,
                                                 std::function<void(void *)> mem_copy_callback) {
    auto vBuffer = create_vertex_index_buffer(handle, size);
    if (vBuffer->empty())
        return {};
    // 没有创建成功，直接退出
    // 创建成功，之后，记录，还是？

    if (vBuffer->host_visible() == false) {
        LOG_INFO(g_log(), "can find a cpu write memory, only get GPU memory", size);
        auto staging_buffer = create_staging_buffer(handle, size);
        if (staging_buffer->empty()) {
            // 创建 staging_buffer 失败
            vBuffer->DestroyBuffer();
            return {};
        }

        if (staging_buffer->host_visible() == false) {
            LOG_INFO(g_log(), "can find a cpu write memory, allocate size {}", size);
        } else {
            copy_mem_from_cpu_to_gpu(staging_buffer, mem_copy_callback);
            copy_vk_buffer_and_execution(handle, staging_buffer, vBuffer, size);
        }
        staging_buffer->DestroyBuffer();
    } else {
        // 创建成功，但是 map 不成功的很少见
        copy_mem_from_cpu_to_gpu(vBuffer, mem_copy_callback);
    }
    return vBuffer;
}


inline Model_mesh create_mesh_data(const VK_handle &handle, const vertex_and_attributes &vertices,
                                   const Indices_type &indices_) {
    VkDeviceSize vBufSize{vertices.size};
    VkDeviceSize iBufSize{sizeof(uint16_t) * indices_->size()};

    // 具体的复制函数
    auto mem_copy_function = [vertices,vBufSize,indices_,iBufSize](void *dst) {
        memcpy(dst, vertices.data, vBufSize);
        memcpy(static_cast<char *>(dst) + vBufSize, indices_->data(), iBufSize);
    };

    const auto vertices_buffer =
            create_vertex_index_buffer(handle, vBufSize + iBufSize, mem_copy_function);


    Model_mesh mesh;
    mesh.vertices = vertices_buffer;
    mesh.indices  = vertices_buffer;
    // mesh.indices_offset = vBufSize;
    mesh.indexed_command.indexCount    = indices_->size(); // 是可以这么替换的
    mesh.indexed_command.firstIndex    = vBufSize / 2;     // 索引缓冲区的起始偏移（以索引为单位）确实是可以通过计算偏移的
    mesh.indexed_command.vertexOffset  = 0;
    mesh.indexed_command.instanceCount = 1;
    mesh.indexed_command.firstInstance = 0;


    return mesh;
}


inline Model_mesh create_mesh(const VK_handle &handle, logic_render_data &data,
                              std::map<logic_render_data *, mesh_and_share> &map) {
    auto it = map.find(&data);
    if (it != map.end()) {
        it->second.shared_number++;
        return it->second.mesh;
    } else {
        if (data.mesh_path_.empty() == false) {
            auto [vertices, indices] = load_model(data.mesh_path_);
            const auto mesh          = create_mesh_data(handle, vertices, indices);
            // map.insert({data, {mesh, 1}});
            return mesh;
        } else {
            for (const auto &temp: data.vertex_and_attributes_) {
                // create_vertex_buffer(temp.shared_ptr_of_vertices_, temp.size, temp.data, &vertices_map_);
                auto mesh = create_mesh_data(handle, temp, data.indices_);
                // map.insert({data, {mesh, 1}});
                return mesh;
            }
        }
    }
}


inline Model_mesh *find_mesh(logic_render_data data,
                             std::map<logic_render_data *, mesh_and_share> &map) {
    auto it = map.find(&data);
    if (it != map.end()) {
        return &it->second.mesh;
    } else {
        return nullptr;
    }
}

inline void clean_all_mesh_object(VK_handle &handle) {
    // 正式项目中，确保 vkDeviceWaitIdle 后按顺序销毁资源是专业开发者的标准做法
    for (const auto &[key, value]: VK_handle::get().get_mesh_map()) {
        value.mesh.vertices->DestroyBuffer();
        // ->不清理会直接爆异常
    }
    VK_handle::get().get_mesh_map().clear();
    // discard_buffer_map_clean();
}
#endif //HOWTOVULKAN_VERTEX_AND_BUFFER_INDEX_H
