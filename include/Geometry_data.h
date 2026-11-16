//
// Created by 潘鑫 on 2026/5/24.
//

#ifndef HELLO_MAC_GEOMETRY_DATA_H
#define HELLO_MAC_GEOMETRY_DATA_H
#include <vector>
#include "AABB_box.h"
#include "Eigen/Eigen"

struct Vertex_2D {
    Eigen::Vector2f pos{0, 0};
    Eigen::Vector2f uv{0, 0};
};

struct Vertex_imgui {
    Eigen::Vector2f pos{0, 0};
    Eigen::Vector2f uv{0, 0};
    uint32_t color = 0xffffffff;
};


struct share_block {
    std::shared_ptr<void> ptr = nullptr;
    void *data                = nullptr;
    size_t total_size         = 0;
    size_t count              = 0;
    size_t single_size        = 0;
    // std::vector<VertexAttrib> vertex_attribs;  // 这里暂时清除了


    template<typename H>
    friend H AbslHashValue(H state, const share_block &sp) {
        // 直接使用 H::combine 把所有成员丢进去，它支持任意数量、任意类型的参数！
        // 并且完美支持原生枚举（如 VkPrimitiveTopology），不需要进行 static_cast 转换
        return H::combine(std::move(state),
                          sp.ptr,
                          sp.data,
                          sp.total_size,
                          sp.count,
                          sp.single_size);
    }
};


struct Geometry_data_need_copy_tag {
};

struct Tag_bindless_batch {
};



class Geometry {
    share_block vertices_;
    share_block indices_;
    std::size_t materials_ = 0;
    Local_Space_AABB AABBs_;
};


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

    [[nodiscard]] std::vector<share_block> &get_vertices() {
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


    bool add_geometry(const Geometry_data &data) {
        for (auto &vertex: data.get_vertices()) {
            vertices_.push_back(vertex);
        }
        for (auto &index: data.get_indices()) {
            indices_.push_back(index);
        }
        for (auto &material: data.get_materials()) {
            materials_.push_back(material);
        }
        for (auto &aabb: data.get_aabbs()) {
            AABBs_.push_back(aabb);
        }
        return true;
    }

private:
    std::vector<share_block> vertices_;
    std::vector<share_block> indices_;
    std::vector<std::size_t> materials_;
    std::vector<Local_Space_AABB> AABBs_;
};

inline size_t get_size(const std::vector<share_block> &blocks) {
    size_t buf_size = 0;
    for (const auto &vertex: blocks) {
        buf_size += vertex.total_size;
    }
    return buf_size;
}

inline void append_text_box(const std::shared_ptr<std::vector<Vertex_2D> > &vertices,
                            const std::shared_ptr<std::vector<unsigned short> > &indices,
                            Eigen::Vector2f min, Eigen::Vector2f max,
                            const float uv_min_x, const float uv_min_y, const float uv_max_x, const float uv_max_y) {
    indices->push_back(vertices->size() + 0);
    indices->push_back(vertices->size() + 1);
    indices->push_back(vertices->size() + 2);
    indices->push_back(vertices->size() + 2);
    indices->push_back(vertices->size() + 3);
    indices->push_back(vertices->size() + 0);
    //     3            2
    //      ************
    //      *        * *
    //      *     *    *
    //      *  *       *
    //      ************
    //     0            1
    vertices->emplace_back(Vertex_2D{{min.x(), min.y()}, {uv_min_x, uv_min_y}}); //0 1 2
    vertices->emplace_back(Vertex_2D{{max.x(), min.y()}, {uv_max_x, uv_min_y}});
    vertices->emplace_back(Vertex_2D{{max.x(), max.y()}, {uv_max_x, uv_max_y}}); // 2 3 0
    vertices->emplace_back(Vertex_2D{{min.x(), max.y()}, {uv_min_x, uv_max_y}});
}

inline void append_text_box(const std::shared_ptr<std::vector<Vertex_2D> > &vertices,
                            const std::shared_ptr<std::vector<unsigned short> > &indices,
                            Eigen::Vector2f min, Eigen::Vector2f max,
                            Eigen::Vector4f uv) {
    indices->push_back(vertices->size() + 0);
    indices->push_back(vertices->size() + 1);
    indices->push_back(vertices->size() + 2);
    indices->push_back(vertices->size() + 2);
    indices->push_back(vertices->size() + 3);
    indices->push_back(vertices->size() + 0);
    //     3            2
    //      ************
    //      *        * *
    //      *     *    *
    //      *  *       *
    //      ************
    //     0            1
    vertices->emplace_back(Vertex_2D{{min.x(), min.y()}, {uv.x(), uv.y()}}); //0 1 2
    vertices->emplace_back(Vertex_2D{{max.x(), min.y()}, {uv.z(), uv.y()}});
    vertices->emplace_back(Vertex_2D{{max.x(), max.y()}, {uv.z(), uv.w()}}); // 2 3 0
    vertices->emplace_back(Vertex_2D{{min.x(), max.y()}, {uv.x(), uv.w()}});
    return;
}


#endif //HELLO_MAC_GEOMETRY_DATA_H
