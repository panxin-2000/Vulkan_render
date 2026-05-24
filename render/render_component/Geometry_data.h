//
// Created by 潘鑫 on 2026/5/24.
//

#ifndef HELLO_MAC_GEOMETRY_DATA_H
#define HELLO_MAC_GEOMETRY_DATA_H
#include "APP_utility_mixins.h"
#include "vector"
#include "global_singleton.h"
#include "shader_common.h"

struct share_block {
    std::shared_ptr<void> ptr;
    void *data;
    size_t total_size;
    size_t count;
    size_t single_size;
    // std::vector<VertexAttrib> vertex_attribs;  // 这里暂时清除了
};

/**
 * 目的是两个各种各样格式的数据 放置到 vertex 和 index 两个 共享指针的内存区中
 */
class Geometry_data : public NonCopyable {
public:
    Geometry_data() = default;

    ~Geometry_data() = default;

    void set_vertices(const share_block &temp) {
        vertices_ = temp;
    }

    void set_indices(const share_block &indices) {
        indices_ = indices;
    }

    [[nodiscard]] share_block get_vertices() const {
        return vertices_;
    };

    [[nodiscard]] share_block get_indices() const {
        return indices_;
    };

    template<typename vertex_t, typename index_t>
    void set(const std::shared_ptr<std::vector<vertex_t> > &sp_vertices,
             const std::shared_ptr<std::vector<index_t> > &sp_indices) {
        if (sp_vertices != nullptr) {
            const share_block vertices_buffer = {
                sp_vertices,
                sp_vertices->data(),
                sp_vertices->size() * sizeof(vertex_t),
                sp_vertices->size(),
                sizeof(vertex_t)
            };
            vertices_ = vertices_buffer;
        }

        if (sp_indices != nullptr) {
            const share_block indices_buffer = {
                sp_indices,
                sp_indices->data(),
                sp_indices->size() * sizeof(index_t),
                sp_indices->size(),
                sizeof(index_t)
            };
            indices_ = indices_buffer;
        }
    }

private:
    share_block vertices_;
    share_block indices_;
};

template<typename vertex_t, typename index_t>
void add_geometry_data(const entt::entity entity,
                       const std::shared_ptr<std::vector<vertex_t> > &sp_vertices,
                       const std::shared_ptr<std::vector<index_t> > &sp_indices) {
    if (auto *pos = Logic_entt().try_get<Geometry_data>(entity)) {
        Logic_entt().remove<Geometry_data>(entity);
    }
    Logic_entt().emplace<Geometry_data>(entity);

    auto &geometry = Logic_entt().get<Geometry_data>(entity);

    geometry.set(sp_vertices, sp_indices);
}

bool add_2D_bound_box_geometry(const entt::entity entity,
                               const Point_2 min,
                               const Point_2 max);

bool add_2D_bound_box_geometry(const entt::entity entity,
                               const Point_3 min,
                               const Point_3 max);

bool add_round_box_geometry(entt::entity entity,
                            Point_3 min,
                            Point_3 max);

bool add_triangle_geometry(entt::entity entity,
                           Point_3 a,
                           Point_3 b,
                           Point_3 c);

void append_text_box(const std::shared_ptr<std::vector<Vertex> > &vertices,
                     const std::shared_ptr<std::vector<unsigned short> > &indices,
                     Point_3 min, Point_3 max,
                     float uv_min_x, float uv_min_y, float uv_max_x, float uv_max_y);

bool add_sky_box_data(entt::entity entity);

inline AABB_min_max<Point_3> find_min_max_point(const std::shared_ptr<std::vector<Vertex> > vertices) {
    Point_3 min = Point_3::init_max_limit();
    Point_3 max = Point_3::init_min_limit();
    for (auto &vertex: *vertices) {
        min = Point_3::min_two_point(vertex.pos, min);
        max = Point_3::max_two_point(vertex.pos, max);
    }
    return {min, max};
}

#endif //HELLO_MAC_GEOMETRY_DATA_H
