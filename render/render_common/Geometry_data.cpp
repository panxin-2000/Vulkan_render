//
// Created by 潘鑫 on 2026/5/24.
//

#include "Geometry_data.h"
#include "../render_component/shader_component.h"


bool add_2D_bound_box_geometry(const entt::entity entity,
                               const Point_2 min,
                               const Point_2 max) {
    const auto vertices = std::make_shared<std::vector<Vertex_2D> >(); //  32  * 4 = 128
    const auto indices  = std::make_shared<std::vector<uint16_t> >();  //  2   * 6 = 12
    {
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
        vertices->emplace_back(Vertex_2D{{min.x, min.y}, 0, 0}); //0 1 2
        vertices->emplace_back(Vertex_2D{{max.x, min.y}, 1, 0});
        vertices->emplace_back(Vertex_2D{{max.x, max.y}, 1, 1}); // 2 3 0
        vertices->emplace_back(Vertex_2D{{min.x, max.y}, 0, 1});
    }
    add_geometry_data(entity, vertices, indices);
    return true;
}

bool add_2D_bound_box_geometry(const entt::entity entity,
                               const Point_3 min,
                               const Point_3 max) {
    const auto vertices = std::make_shared<std::vector<Vertex> >();   //  32  * 4 = 128
    const auto indices  = std::make_shared<std::vector<uint16_t> >(); //  2   * 6 = 12
    // 要改这里，需要改的内容似乎就有点说了，之后再看看怎么改吧。
    {
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
        vertices->emplace_back(Vertex{{min.x, min.y, min.z}, {0, 0, 0}, {0, 0}}); //0 1 2
        vertices->emplace_back(Vertex{{max.x, min.y, min.z}, {0, 0, 0}, {1, 0}});
        vertices->emplace_back(Vertex{{max.x, max.y, max.z}, {0, 0, 0}, {1, 1}}); // 2 3 0
        vertices->emplace_back(Vertex{{min.x, max.y, max.z}, {0, 0, 0}, {0, 1}});
    }
    add_geometry_data(entity, vertices, indices);
    return true;
}

bool add_round_box_geometry(const entt::entity entity,
                            Point_3 min,
                            Point_3 max) {
    auto vertex_input   = get_attribute_description(entity);
    const auto vertices = std::make_shared<std::vector<Vertex> >();   //  32  * 4 = 128
    const auto indices  = std::make_shared<std::vector<uint16_t> >(); //  2   * 6 = 12
    // 要改这里，需要改的内容似乎就有点说了，之后再看看怎么改吧。
    {
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
        vertices->emplace_back(Vertex{{min.x, min.y, min.z}, {0, 0, 0}, {min.x, min.y}}); //0 1 2
        vertices->emplace_back(Vertex{{max.x, min.y, min.z}, {0, 0, 0}, {max.x, min.y}});
        vertices->emplace_back(Vertex{{max.x, max.y, max.z}, {0, 0, 0}, {max.x, max.y}}); // 2 3 0
        vertices->emplace_back(Vertex{{min.x, max.y, max.z}, {0, 0, 0}, {min.x, max.y}});
    }

    add_geometry_data(entity, vertices, indices);
    return true;
}

bool add_triangle_geometry(const entt::entity entity,
                           Point_3 a,
                           Point_3 b,
                           Point_3 c) {
    const auto vertices = std::make_shared<std::vector<Vertex> >();   //  32  * 3 = 96
    const auto indices  = std::make_shared<std::vector<uint16_t> >(); //  2   * 3 = 6
    // 要改这里，需要改的内容似乎就有点说了，之后再看看怎么改吧。
    {
        indices->push_back(vertices->size() + 0);
        indices->push_back(vertices->size() + 1);
        indices->push_back(vertices->size() + 2);
        vertices->emplace_back(Vertex{{a.x, a.y, a.z}, {0, 0, 0}, {0, 0}}); //0 1 2
        vertices->emplace_back(Vertex{{b.x, b.y, b.z}, {0, 0, 0}, {0, 0}}); //0 1 2
        vertices->emplace_back(Vertex{{c.x, c.y, c.z}, {0, 0, 0}, {0, 0}}); //0 1 2
    }
    add_geometry_data(entity, vertices, indices);
    return true;
}

void clean_geometry_data(const entt::entity entity) {
    if (Logic_entt().try_get<Geometry_data>(entity)) {
        Logic_entt().remove<Geometry_data>(entity);
    }
}


bool add_path(const entt::entity entity, const std::vector<Eigen::Vector2f> &path, const Color color) {
    const auto vertices   = std::make_shared<std::vector<Line> >();
    const auto indices    = std::make_shared<std::vector<uint16_t> >();
    const uint8_t color_r = static_cast<uint8_t>(std::clamp(color.R * 255.0f + 0.5f, 0.0f, 255.0f));
    const uint8_t color_g = static_cast<uint8_t>(std::clamp(color.G * 255.0f + 0.5f, 0.0f, 255.0f));
    const uint8_t color_b = static_cast<uint8_t>(std::clamp(color.B * 255.0f + 0.5f, 0.0f, 255.0f));
    const uint8_t color_a = static_cast<uint8_t>(std::clamp(color.LightType * 255.0f + 0.5f, 0.0f, 255.0f));
    for (const auto &vertex: path) {
        vertices->emplace_back(Line{{vertex.x(), vertex.y()}, color_r, color_g, color_b, color_a});
    }
    for (uint i = 0; i < path.size() - 1; ++i) {
        indices->push_back(i + 0);
        indices->push_back(i + 1);
    }
    add_geometry_data(entity, vertices, indices);
    return true;
}


bool add_box_data(const entt::entity entity,
                  const float x_min,
                  const float y_min,
                  const float z_min,
                  const float x_max,
                  const float y_max,
                  const float z_max) {
    const auto vertices = std::make_shared<std::vector<Vertex> >();   //  32  * 4 = 128
    const auto indices  = std::make_shared<std::vector<uint16_t> >(); //  2   * 6 = 12
    indices->push_back(vertices->size() + 0);
    indices->push_back(vertices->size() + 1);
    indices->push_back(vertices->size() + 2);

    indices->push_back(vertices->size() + 3);
    indices->push_back(vertices->size() + 2);
    indices->push_back(vertices->size() + 1);

    indices->push_back(vertices->size() + 4);
    indices->push_back(vertices->size() + 5);
    indices->push_back(vertices->size() + 6);

    indices->push_back(vertices->size() + 7);
    indices->push_back(vertices->size() + 6);
    indices->push_back(vertices->size() + 5);

    indices->push_back(vertices->size() + 8);
    indices->push_back(vertices->size() + 9);
    indices->push_back(vertices->size() + 10);

    indices->push_back(vertices->size() + 11);
    indices->push_back(vertices->size() + 10);
    indices->push_back(vertices->size() + 9);

    indices->push_back(vertices->size() + 12);
    indices->push_back(vertices->size() + 13);
    indices->push_back(vertices->size() + 14);

    indices->push_back(vertices->size() + 15);
    indices->push_back(vertices->size() + 14);
    indices->push_back(vertices->size() + 13);

    indices->push_back(vertices->size() + 16);
    indices->push_back(vertices->size() + 17);
    indices->push_back(vertices->size() + 18);

    indices->push_back(vertices->size() + 19);
    indices->push_back(vertices->size() + 18);
    indices->push_back(vertices->size() + 17);

    indices->push_back(vertices->size() + 20);
    indices->push_back(vertices->size() + 21);
    indices->push_back(vertices->size() + 22);

    indices->push_back(vertices->size() + 23);
    indices->push_back(vertices->size() + 22);
    indices->push_back(vertices->size() + 21);

    Point_2 UV_min{0.0, 0.0};
    Point_2 UV_max{1.0, 1.0};

    //                              * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    //                           *  | E                                                    *  | F
    //                        *     |                                                  *      |
    //                     *        |                                                *        |
    //                  *           |                                             *           |
    //               *              |                                          *              |
    //            *                 |                                       *                 |
    //         *                    |                                    *                    |
    //      * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *                       |
    //      *   A                   |                             B   *                       |
    //      *                       |                                 *                       |
    //      *                       |                                 *                       |
    //      *                       |                                 *                       |
    //      *                       |                                 *                       |
    //      *                       |                                 *                       |
    //      *                       |                                 *                       |
    //      *                       |                                 *                       |
    //      *                       |                                 *                       |
    //      *                       |                                 *                       |
    //      *                       |                                 *                       |
    //      *                       |  G                              *                       | H
    //      *                       ~---------------------------------*------------------------
    //      *                    ~                                    *                   ~
    //      *                 ~                                       *                 ~
    //      *              ~                                          *              ~
    //      *           ~                                             *           ~
    //      *        ~                                                *        ~
    //      *     ~                                                   *     ~
    //      *  ~                                                      *  ~
    //      * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    //      C                                                         D
    //
    //
    Point_3 C{x_min, y_min, z_max};
    Point_3 D{x_max, y_min, z_max};
    Point_3 A{x_min, y_max, z_max};
    Point_3 B{x_max, y_max, z_max};
    Point_3 H{x_max, y_min, z_min};
    Point_3 G{x_min, y_min, z_min};
    Point_3 F{x_max, y_max, z_min};
    Point_3 E{x_min, y_max, z_min};


    // vulkan 右手坐标系  主要 是需要 注意 与 贴图 坐标系的 关联
    // 前面
    vertices->emplace_back(Vertex{C, {0, 0, 1}, {UV_min.x, UV_max.y}});
    vertices->emplace_back(Vertex{D, {0, 0, 1}, {UV_max.x, UV_min.y}});
    vertices->emplace_back(Vertex{A, {0, 0, 1}, {UV_min.x, UV_max.y}});
    vertices->emplace_back(Vertex{B, {0, 0, 1}, {UV_max.x, UV_min.y}});

    // 下面
    vertices->emplace_back(Vertex{G, {0, -1, 0}, {UV_min.x, UV_max.y}});
    vertices->emplace_back(Vertex{H, {0, -1, 0}, {UV_max.x, UV_min.y}});
    vertices->emplace_back(Vertex{C, {0, -1, 0}, {UV_min.x, UV_max.y}});
    vertices->emplace_back(Vertex{D, {0, -1, 0}, {UV_max.x, UV_min.y}});
    //
    // // 右面
    vertices->emplace_back(Vertex{D, {1, 0, 0}, {UV_min.x, UV_max.y}});
    vertices->emplace_back(Vertex{H, {1, 0, 0}, {UV_max.x, UV_min.y}});
    vertices->emplace_back(Vertex{B, {1, 0, 0}, {UV_min.x, UV_max.y}});
    vertices->emplace_back(Vertex{F, {1, 0, 0}, {UV_max.x, UV_min.y}});
    //
    // // 上面
    vertices->emplace_back(Vertex{A, {0, 1, 0}, {UV_min.x, UV_max.y}});
    vertices->emplace_back(Vertex{B, {0, 1, 0}, {UV_max.x, UV_min.y}});
    vertices->emplace_back(Vertex{E, {0, 1, 0}, {UV_min.x, UV_max.y}});
    vertices->emplace_back(Vertex{F, {0, 1, 0}, {UV_max.x, UV_min.y}});
    //
    // // 左面
    vertices->emplace_back(Vertex{G, {-1, 0, 0}, {UV_min.x, UV_max.y}});
    vertices->emplace_back(Vertex{C, {-1, 0, 0}, {UV_max.x, UV_min.y}});
    vertices->emplace_back(Vertex{E, {-1, 0, 0}, {UV_min.x, UV_max.y}});
    vertices->emplace_back(Vertex{A, {-1, 0, 0}, {UV_max.x, UV_min.y}});
    //
    // // 后面                                                                 UV
    vertices->emplace_back(Vertex{H, {0, 0, -1}, {UV_min.x, UV_max.y}});
    vertices->emplace_back(Vertex{G, {0, 0, -1}, {UV_max.x, UV_min.y}});
    vertices->emplace_back(Vertex{F, {0, 0, -1}, {UV_min.x, UV_max.y}});
    vertices->emplace_back(Vertex{E, {0, 0, -1}, {UV_max.x, UV_min.y}});
    add_geometry_data(entity, vertices, indices);
    return true;
}

bool add_box_data(const entt::entity entity, const AABB_min_max<Point_3> &bounding_box) {
    return add_box_data(entity,
                        bounding_box.min_point_.x,
                        bounding_box.min_point_.y,
                        bounding_box.min_point_.z,
                        bounding_box.max_point_.x,
                        bounding_box.max_point_.y,
                        bounding_box.max_point_.z);
}

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
    const auto temp                  = (min + max) / 2;
    const auto temp_2                = (max - min) / 2;
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
    const auto temp                  = (min + max) / 2;
    const auto temp_2                = (max - min) / 2;
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
