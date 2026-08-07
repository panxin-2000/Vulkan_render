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


struct Geometry_data_need_copy_tag {
};

/**
 * 目的是两个各种各样格式的数据 放置到 vertex 和 index 两个 共享指针的内存区中
 */
class Geometry_data {
public:
    Geometry_data() = default;

    ~Geometry_data() = default;

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

    geometry.push(sp_vertices, sp_indices);
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

void append_text_box(const std::shared_ptr<std::vector<Vertex_2D> > &vertices,
                     const std::shared_ptr<std::vector<unsigned short> > &indices,
                     Point_2 min, Point_2 max,
                     float uv_min_x, float uv_min_y, float uv_max_x, float uv_max_y);


bool add_box_data(entt::entity entity, const AABB_min_max<Point_3> &bounding_box);

bool add_box_data(entt::entity entity,
                  float x_min = -0.5,
                  float y_min = -0.5,
                  float z_min = -0.5,
                  float x_max = 0.5,
                  float y_max = 0.5,
                  float z_max = 0.5);

inline AABB_min_max<Point_3> find_min_max_point(const std::shared_ptr<std::vector<Vertex> > vertices) {
    Point_3 min = Point_3::init_max_limit();
    Point_3 max = Point_3::init_min_limit();
    for (auto &vertex: *vertices) {
        min = Point_3::min(vertex.pos, min);
        max = Point_3::max(vertex.pos, max);
    }
    return {min, max};
}

inline AABB_min_max<Point_3> find_min_max_point(const std::vector<share_block> &vertices) {
    Point_3 min = Point_3::init_max_limit();
    Point_3 max = Point_3::init_min_limit();
    for (auto vertex: vertices) {
        for (int i = 0; i < vertex.count; i++) {
            // 有一个大的前提，那就是 默认 位置一定是 pos 是在最前的
            auto pos = reinterpret_cast<Point_3 *>(static_cast<char *>(vertex.data) + vertex.single_size * i);
            min      = Point_3::min(*pos, min);
            max      = Point_3::max(*pos, max);
        }
    }
    return {min, max};
}

// inline AABB_min_max<Point_3> find_min_max_point(const share_block &vertex) {
//     Point_3 min = Point_3::init_max_limit();
//     Point_3 max = Point_3::init_min_limit();
//     for (int i = 0; i < vertex.count; i++) {
//         // 有一个大的前提，那就是 默认 位置一定是 pos 是在最前的
//         auto pos = reinterpret_cast<Point_3 *>(static_cast<char *>(vertex.data) + vertex.single_size * i);
//         min      = Point_3::min(*pos, min);
//         max      = Point_3::max(*pos, max);
//     }
//     return {min, max};
// }

struct alignas(16) Render_AABB {
    Eigen::Vector4f centroid_points;
    Eigen::Vector4f direction_intervals;
};


inline Render_AABB find_min_max_point(const share_block &vertex) {
    Eigen::Vector3f min = Eigen::Vector3f::Constant(std::numeric_limits<float>::infinity());;
    Eigen::Vector3f max = Eigen::Vector3f::Constant(-std::numeric_limits<float>::infinity());
    for (int i = 0; i < vertex.count; i++) {
        // 有一个大的前提，那就是 默认 位置一定是 pos 是在最前的
        Eigen::Map<Eigen::Vector3f> pos(reinterpret_cast<float *>(
                                            static_cast<char *>(vertex.data) + vertex.single_size * i));
        min = min.cwiseMin(pos);
        max = max.cwiseMax(pos);
    }
    Render_AABB bounding_box;
    auto temp                        = (min + max) / 2;
    auto temp_2                      = (max - min) / 2;
    bounding_box.centroid_points     = {temp.x(), temp.y(), temp.y(), 1.0f};
    bounding_box.direction_intervals = {temp_2.x(), temp_2.y(), temp_2.y(), 0.0f};
    return bounding_box;
}

#endif //HELLO_MAC_GEOMETRY_DATA_H
