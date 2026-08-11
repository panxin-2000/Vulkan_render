//
// Created by 潘鑫 on 2026/5/24.
//

#include "Geometry_data.h"

#include "bezier_curve.h"
#include "shader_component.h"


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

bool add_round_box_geometry(entt::entity entity,
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
}

bool add_triangle_geometry(entt::entity entity,
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
}

bool add_line(entt::entity entity,
              Point_2 a,
              Point_2 b) {
    const auto vertices = std::make_shared<std::vector<Line> >();
    const auto indices  = std::make_shared<std::vector<uint16_t> >();
    // 要改这里，需要改的内容似乎就有点说了，之后再看看怎么改吧。
    {
        indices->push_back(vertices->size() + 0);
        indices->push_back(vertices->size() + 1);
        vertices->emplace_back(Line{{a.x, a.y}, 1, 0, 0, 0});
        vertices->emplace_back(Line{{b.x, b.y}, 255, 0, 0, 0});
    }
    add_geometry_data(entity, vertices, indices);
}

bool add_bezier(entt::entity entity) {
    Bezier<Eigen::Vector2f> bezier({200, 200}, {200, 600}, {600, 200}, {600, 600}, 1.25);
    std::vector<Eigen::Vector2f> path;
    bezier.Casteljau(&path);
    const auto vertices = std::make_shared<std::vector<Line> >();
    const auto indices  = std::make_shared<std::vector<uint16_t> >();
    for (const auto &vertex: path) {
        vertices->emplace_back(Line{{vertex.x(), vertex.y()}, 1, 0, 0, 255});
    }
    for (uint i = 0; i < path.size() - 1; ++i) {
        indices->push_back(i + 0);
        indices->push_back(i + 1);
    }
    add_geometry_data(entity, vertices, indices);
}

bool add_box_data(entt::entity entity, const AABB_min_max<Point_3> &bounding_box) {
    return add_box_data(entity,
                        bounding_box.min_point_.x,
                        bounding_box.min_point_.y,
                        bounding_box.min_point_.z,
                        bounding_box.max_point_.x,
                        bounding_box.max_point_.y,
                        bounding_box.max_point_.z);
}

bool add_box_data(entt::entity entity,
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
}
