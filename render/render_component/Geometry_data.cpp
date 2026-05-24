//
// Created by 潘鑫 on 2026/5/24.
//

#include "Geometry_data.h"

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
        vertices->emplace_back(Vertex{{min.x, min.y, min.z}, 0, 0, 0, 0, 0}); //0 1 2
        vertices->emplace_back(Vertex{{max.x, min.y, min.z}, 0, 0, 0, 1, 0});
        vertices->emplace_back(Vertex{{max.x, max.y, max.z}, 0, 0, 0, 1, 1}); // 2 3 0
        vertices->emplace_back(Vertex{{min.x, max.y, max.z}, 0, 0, 0, 0, 1});
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
        vertices->emplace_back(Vertex{{min.x, min.y, min.z}, 0, 0, 0, min.x, min.y}); //0 1 2
        vertices->emplace_back(Vertex{{max.x, min.y, min.z}, 0, 0, 0, max.x, min.y});
        vertices->emplace_back(Vertex{{max.x, max.y, max.z}, 0, 0, 0, max.x, max.y}); // 2 3 0
        vertices->emplace_back(Vertex{{min.x, max.y, max.z}, 0, 0, 0, min.x, max.y});
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

bool add_sky_box_data(entt::entity entity) {
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
    vertices->emplace_back(Vertex{{-0.5, -0.5, 0.5}, {0, 0, 1}, {0, 0}});
    vertices->emplace_back(Vertex{{0.5, -0.5, 0.5}, {0, 0, 1}, {0, 0}});
    vertices->emplace_back(Vertex{{-0.5, 0.5, 0.5}, {0, 0, 1}, {0, 0}});
    vertices->emplace_back(Vertex{{0.5, 0.5, 0.5}, {0, 0, 1}, {0, 0}});
    vertices->emplace_back(Vertex{{0.5, -0.5, 0.5}, {0, -1, 0}, {0, 0}});
    vertices->emplace_back(Vertex{{-0.5, -0.5, 0.5}, {0, -1, 0}, {0, 0}});
    vertices->emplace_back(Vertex{{0.5, -0.5, -0.5}, {0, -1, 0}, {0, 0}});
    vertices->emplace_back(Vertex{{-0.5, -0.5, -0.5}, {0, -1, 0}, {0, 0}});
    vertices->emplace_back(Vertex{{0.5, 0.5, 0.5}, {1, 0, 0}, {0, 0}});
    vertices->emplace_back(Vertex{{0.5, -0.5, 0.5}, {1, 0, 0}, {0, 0}});
    vertices->emplace_back(Vertex{{0.5, 0.5, -0.5}, {1, 0, 0}, {0, 0}});
    vertices->emplace_back(Vertex{{0.5, -0.5, -0.5}, {1, 0, 0}, {0, 0}});
    vertices->emplace_back(Vertex{{-0.5, 0.5, 0.5}, {0, 1, 0}, {0, 0}});
    vertices->emplace_back(Vertex{{0.5, 0.5, 0.5}, {0, 1, 0}, {0, 0}});
    vertices->emplace_back(Vertex{{-0.5, 0.5, -0.5}, {0, 1, 0}, {0, 0}});
    vertices->emplace_back(Vertex{{0.5, 0.5, -0.5}, {0, 1, 0}, {0, 0}});
    vertices->emplace_back(Vertex{{-0.5, -0.5, 0.5}, {-1, 0, 0}, {0, 0}});
    vertices->emplace_back(Vertex{{-0.5, 0.5, 0.5}, {-1, 0, 0}, {0, 0}});
    vertices->emplace_back(Vertex{{-0.5, -0.5, -0.5}, {-1, 0, 0}, {0, 0}});
    vertices->emplace_back(Vertex{{-0.5, 0.5, -0.5}, {-1, 0, 0}, {0, 0}});
    vertices->emplace_back(Vertex{{-0.5, -0.5, -0.5}, {0, 0, -1}, {0, 0}});
    vertices->emplace_back(Vertex{{-0.5, 0.5, -0.5}, {0, 0, -1}, {0, 0}});
    vertices->emplace_back(Vertex{{0.5, -0.5, -0.5}, {0, 0, -1}, {0, 0}});
    vertices->emplace_back(Vertex{{0.5, 0.5, -0.5}, {0, 0, -1}, {0, 0}});
    add_geometry_data(entity, vertices, indices);
    return true;
}


void append_text_box(const std::shared_ptr<std::vector<Vertex> > &vertices,
                     const std::shared_ptr<std::vector<unsigned short> > &indices,
                     Point_3 min, Point_3 max,
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
    vertices->emplace_back(Vertex{{min.x, min.y, min.z}, 0, 0, 0, uv_min_x, uv_min_y}); //0 1 2
    vertices->emplace_back(Vertex{{max.x, min.y, min.z}, 0, 0, 0, uv_max_x, uv_min_y});
    vertices->emplace_back(Vertex{{max.x, max.y, max.z}, 0, 0, 0, uv_max_x, uv_max_y}); // 2 3 0
    vertices->emplace_back(Vertex{{min.x, max.y, max.z}, 0, 0, 0, uv_min_x, uv_max_y});
}
