//
// Created by 潘鑫 on 2026/5/24.
//

#ifndef HELLO_MAC_GEOMETRY_DATA_H
#define HELLO_MAC_GEOMETRY_DATA_H
#include <oneapi/tbb.h>

#include "vector"
#include "global_singleton.h"
#include "PBR_component.h"
#include "shader_common.h"
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


Render_AABB find_min_max_point(const share_block &vertex);

/**
 * 目的是两个各种各样格式的数据 放置到 vertex 和 index 两个 共享指针的内存区中
 */
class Geometry_data {
public:
    Geometry_data() = default;

    ~Geometry_data() = default;

    template<typename vertex_t>
    void push_vertices(const std::shared_ptr<std::vector<vertex_t> > &sp_vertices, const Render_AABB &aabb) {
        if (sp_vertices != nullptr) {
            const share_block vertices_buffer = {
                sp_vertices,
                sp_vertices->data(),
                sp_vertices->size() * sizeof(vertex_t),
                sp_vertices->size(),
                sizeof(vertex_t)
            };
            vertices_.push_back(vertices_buffer);
            AABBs_.push_back(aabb);
        }
    }

    void push_vertices(const share_block &vertices_buffer) {
        vertices_.push_back(vertices_buffer);
    }

    void push_vertices(const share_block &vertices_buffer, const Render_AABB &aabb) {
        vertices_.push_back(vertices_buffer);
        AABBs_.push_back(aabb);
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

    [[nodiscard]] std::vector<Render_AABB> get_aabbs() const {
        return AABBs_;
    };

    template<typename vertex_t, typename index_t>
    void push(const std::shared_ptr<std::vector<vertex_t> > &sp_vertices,
              const std::shared_ptr<std::vector<index_t> > &sp_indices) {
        if (sp_vertices != nullptr) {
            const share_block vertices_buffer = {
                sp_vertices,
                sp_vertices->data(),
                sp_vertices->size() * sizeof(vertex_t),
                sp_vertices->size(),
                sizeof(vertex_t)
            };
            vertices_.push_back(vertices_buffer);
            const auto bound_box = find_min_max_point(vertices_buffer);
            AABBs_.push_back(bound_box);
        }
        push_indices(sp_indices);
    }

private:
    std::vector<share_block> vertices_;
    std::vector<share_block> indices_;
    std::vector<std::size_t> materials_;
    std::vector<Render_AABB> AABBs_;
};

template<typename vertex_t, typename index_t>
void add_geometry_data(const entt::entity entity,
                       const std::shared_ptr<std::vector<vertex_t> > &sp_vertices,
                       const std::shared_ptr<std::vector<index_t> > &sp_indices) {
    auto &geometry = Logic_entt().get_or_emplace<Geometry_data>(entity);
    geometry.push(sp_vertices, sp_indices);
}

void clean_geometry_data(const entt::entity entity);

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


bool add_path(const entt::entity entity, const std::vector<Eigen::Vector2f> &path, const Color color);

bool add_box_data(const entt::entity entity, const AABB_min_max<Point_3> &bounding_box);

bool add_box_data(entt::entity entity,
                  float x_min = -0.5,
                  float y_min = -0.5,
                  float z_min = -0.5,
                  float x_max = 0.5,
                  float y_max = 0.5,
                  float z_max = 0.5);


inline Render_AABB find_min_max_point(const share_block &vertex) {
    // Eigen::Vector3f min = Eigen::Vector3f::Constant(std::numeric_limits<float>::infinity());;
    // Eigen::Vector3f max = Eigen::Vector3f::Constant(-std::numeric_limits<float>::infinity());
    // for (int i = 0; i < vertex.count; i++) {
    //     // 有一个大的前提，那就是 默认 位置一定是 pos 是在最前的
    //     Eigen::Map<Eigen::Vector3f> pos(reinterpret_cast<float *>(
    //                                         static_cast<char *>(vertex.data) + vertex.single_size * i));
    //     min = min.cwiseMin(pos);
    //     max = max.cwiseMax(pos);
    // }

    // 基础常数定义
    const float inf = std::numeric_limits<float>::infinity();

    struct AABB {
        Eigen::Vector3f min;
        Eigen::Vector3f max;
    };

    // 初始化全域初值
    AABB identity{
        Eigen::Vector3f::Constant(inf),
        Eigen::Vector3f::Constant(-inf)
    };

    // 使用 tbb::parallel_reduce 进行并行化
    auto result = tbb::parallel_reduce(
                                       // 1. 定义迭代范围（建议设置合理粒度，例如 1024 或更大，视顶点数而定）
                                       tbb::blocked_range<int>(0, vertex.count, 2048),

                                       // 2. 身份元素/初始值
                                       identity,

                                       // 3. 线程内部的局部规约（计算局部小分块的 min/max）
                                       [&](const tbb::blocked_range<int> &r, AABB local) -> AABB {
                                           char *base_ptr = static_cast<char *>(vertex.data);
                                           size_t stride  = vertex.single_size;

                                           for (int i = r.begin(); i != r.end(); ++i) {
                                               // 保持您原本的高效内存映射方式
                                               Eigen::Map<const Eigen::Vector3f> pos(
                                                    reinterpret_cast<const float *>(
                                                        base_ptr + stride * i)
                                                   );

                                               local.min = local.min.cwiseMin(pos);
                                               local.max = local.max.cwiseMax(pos);
                                           }
                                           return local;
                                       },

                                       // 4. 跨线程的树状合并（将各个线程的局部 AABB 合并为最终结果）
                                       [](AABB a, AABB b) -> AABB {
                                           return AABB{
                                               a.min.cwiseMin(b.min),
                                               a.max.cwiseMax(b.max)
                                           };
                                       }
                                      );
    const Eigen::Vector3f min = result.min;
    const Eigen::Vector3f max = result.max;


    Render_AABB bounding_box;
    const auto temp                  = (min + max) / 2;
    const auto temp_2                = (max - min) / 2;
    bounding_box.centroid_points     = {temp.x(), temp.y(), temp.z(), 1.0f};
    bounding_box.direction_intervals = {temp_2.x(), temp_2.y(), temp_2.z(), 0.0f};
    return bounding_box;
}

#endif //HELLO_MAC_GEOMETRY_DATA_H
