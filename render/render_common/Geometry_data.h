//
// Created by 潘鑫 on 2026/5/24.
//

#ifndef HELLO_MAC_GEOMETRY_DATA_H
#define HELLO_MAC_GEOMETRY_DATA_H
#include <oneapi/tbb.h>

#include "vector"
#include "global_singleton.h"
#include "../render_component/PBR_component.h"
// #include "shader_common.h"
#include "AABB_box.h"

struct share_block {
    std::shared_ptr<void> ptr;
    void *data;
    size_t total_size;
    size_t count;
    size_t single_size;
    // std::vector<VertexAttrib> vertex_attribs;  // 这里暂时清除了
};


struct Geometry_data_need_copy_tag {
};

struct Mesh_need_create_tag {
};


Render_AABB find_min_max_point(const share_block &vertex);

/**
 * 目的是两个各种各样格式的数据 放置到 vertex 和 index 两个 共享指针的内存区中
 */
class Geometry_data {
public:
    Geometry_data() = default;

    ~Geometry_data() = default;


    void push_AABB(const Local_Space_AABB &aabb) {
        AABBs_.push_back(aabb);
    }


    template<typename vertex_t>
    void push_vertices(const std::shared_ptr<std::vector<vertex_t> > &sp_vertices) {
        if (sp_vertices != nullptr) {
            const share_block vertices_buffer = {
                sp_vertices,
                sp_vertices->data(),
                sp_vertices->size() * sizeof(vertex_t),
                sp_vertices->size(),
                sizeof(vertex_t)
            };
            vertices_.push_back(vertices_buffer);
        }
    }

    void push_vertices(const share_block &vertices_buffer) {
        vertices_.push_back(vertices_buffer);
    }

    void push_vertices(const share_block &vertices_buffer, const Render_AABB &aabb) {
        vertices_.push_back(vertices_buffer);
        AABBs_.push_back(Local_Space_AABB(aabb));
    }

    template<typename index_t>
    void push_indices(const std::shared_ptr<std::vector<index_t> > &sp_indices) {
        if (sp_indices != nullptr) {
            const share_block indices_buffer = {
                sp_indices,
                sp_indices->data(),
                sp_indices->size() * sizeof(index_t),
                sp_indices->size(),
                sizeof(index_t)
            };
            indices_.push_back(indices_buffer);
        }
    }

    void push_indices(const share_block &indices_buffer) {
        indices_.push_back(indices_buffer);
    }

    void push_material(const std::size_t &material) {
        materials_.push_back(material);
    }

    [[nodiscard]] std::vector<share_block> get_vertices() const {
        return vertices_;
    };

    [[nodiscard]] std::vector<std::size_t> get_materials() const {
        return materials_;
    };

    [[nodiscard]] std::vector<share_block> get_indices() const {
        return indices_;
    };

    [[nodiscard]] std::vector<Local_Space_AABB> get_aabbs() const {
        return AABBs_;
    };

    template<typename vertex_t, typename index_t>
    void push(const std::shared_ptr<std::vector<vertex_t> > &sp_vertices,
              const std::shared_ptr<std::vector<index_t> > &sp_indices) {
        push_vertices(sp_vertices);
        push_indices(sp_indices);
    }

private:
    std::vector<share_block> vertices_;
    std::vector<share_block> indices_;
    std::vector<std::size_t> materials_;
    std::vector<Local_Space_AABB> AABBs_;
};


#endif //HELLO_MAC_GEOMETRY_DATA_H
