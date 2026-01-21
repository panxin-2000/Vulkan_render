//
// Created by 潘鑫 on 2025/11/29.
//

#ifndef LEARN_OPENGL_VBO_OBJECT_H
#define LEARN_OPENGL_VBO_OBJECT_H
#include "shader_common.h"
#include <map>


static void bind_vertex_buffer(const Vertices_type &share_point,
                               std::map<Vertices_type, buffer_and_share> *map) {
    if (share_point != nullptr) {
        auto it = map->find(share_point);
        if (it != map->end()) {
            glBindBuffer(GL_ARRAY_BUFFER, it->second.buffer);
        }
    }
}


static void create_vertex_buffer(const Vertices_type &share_point, uint64_t size, void *data,
                                 std::map<Vertices_type, buffer_and_share> *map) {
    if (share_point != nullptr) {
        auto it = map->find(share_point);
        if (it != map->end()) {
            it->second.shared_number++;
        } else {
            unsigned int buffer = 0;
            glGenBuffers(1, &buffer);
            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, NULL_GPU_INDEX);
            map->insert({share_point, {buffer, 1}});
            // 创建VBO
        }
    }
}

static void delete_vertex_buffer(const Vertices_type &share_point,
                                 std::map<Vertices_type, buffer_and_share> *map) {
    if (share_point != nullptr) {
        auto it = map->find(share_point);
        if (it != map->end()) {
            it->second.shared_number--;
            if (it->second.shared_number == 0) {
                glDeleteBuffers(1, &it->second.buffer);
                map->erase(it);
            }
        }
    }
}


static void create_element_buffer(const Indices_type &share_point,
                                  std::map<Indices_type, buffer_and_share> *map) {
    if (share_point != nullptr) {
        auto it = map->find(share_point);
        if (it != map->end()) {
            it->second.shared_number++;
        } else {
            unsigned int buffer = 0;
            glGenBuffers(1, &buffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
            // 设置大小和数量，之后会固定，并上传
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, share_point->size() * sizeof(unsigned int), share_point->data(),
                         GL_STATIC_DRAW);

            // 示例：从第 100 个字节开始，更新 50 字节的数据
            // 只能更新还在固定内存区域内的数据，更新时需要提交提前计算
            // glBufferSubData(GL_ARRAY_BUFFER, 100, 50, newDataPtr);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL_GPU_INDEX);
            map->insert({share_point, {buffer, 1}});
            // 创建EBO
        }
    }
}

static void delete_element_buffer(const Indices_type &share_point,
                                  std::map<Indices_type, buffer_and_share> *map) {
    if (share_point != nullptr) {
        auto it = map->find(share_point);
        if (it != map->end()) {
            it->second.shared_number--;
            if (it->second.shared_number == 0) {
                glDeleteBuffers(1, &it->second.buffer);
                map->erase(it);
            }
        }
    }
}


static void bind_element_buffer(const Indices_type &share_point,
                                std::map<Indices_type, buffer_and_share> *map) {
    if (share_point != nullptr) {
        auto it = map->find(share_point);
        if (it != map->end()) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, it->second.buffer);
        }
    }
}


#endif //LEARN_OPENGL_VBO_OBJECT_H
