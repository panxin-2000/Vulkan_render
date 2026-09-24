//
// Created by 潘鑫 on 2026/5/24.
//
#include "Geometry_data.h"


void append_text_box(const std::shared_ptr<std::vector<Vertex_2D> > &vertices,
                     const std::shared_ptr<std::vector<unsigned short> > &indices,
                     Point_2 min, Point_2 max,
                     float uv_min_x, float uv_min_y, float uv_max_x, float uv_max_y) {
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
    vertices->emplace_back(Vertex_2D{{min.x, min.y}, uv_min_x, uv_min_y}); //0 1 2
    vertices->emplace_back(Vertex_2D{{max.x, min.y}, uv_max_x, uv_min_y});
    vertices->emplace_back(Vertex_2D{{max.x, max.y}, uv_max_x, uv_max_y}); // 2 3 0
    vertices->emplace_back(Vertex_2D{{min.x, max.y}, uv_min_x, uv_max_y});
    return;
}


Render_AABB find_min_max_point_single_thread(const share_block &vertex) {
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
    const auto temp                   = (min + max) / 2;
    const auto temp_2                 = (max - min) / 2;
    bounding_box.centroid_points_     = {temp.x(), temp.y(), temp.z(), 1.0f};
    bounding_box.direction_intervals_ = {temp_2.x(), temp_2.y(), temp_2.z(), 0.0f};
    return bounding_box;
}

Render_AABB find_min_max_point_mult_thread(const share_block &vertex) {
    // 基础常数定义
    constexpr float inf = std::numeric_limits<float>::infinity();

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
    const auto temp                   = (min + max) / 2;
    const auto temp_2                 = (max - min) / 2;
    bounding_box.centroid_points_     = {temp.x(), temp.y(), temp.z(), 1.0f};
    bounding_box.direction_intervals_ = {temp_2.x(), temp_2.y(), temp_2.z(), 0.0f};
    return bounding_box;
}


Render_AABB find_min_max_point(const share_block &vertex) {
    if (vertex.count < 1024 * 4) {
        return find_min_max_point_single_thread(vertex);
    } else {
        return find_min_max_point_mult_thread(vertex);
    }
}
