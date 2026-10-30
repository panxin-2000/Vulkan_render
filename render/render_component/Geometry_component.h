//
// Created by 潘鑫 on 2026/9/24.
//

#ifndef HELLO_MAC_GEOMETRY_COMPONENT_H
#define HELLO_MAC_GEOMETRY_COMPONENT_H


#include "Geometry_data.h"

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

void append_text_box(const std::shared_ptr<std::vector<Vertex_2D> > &vertices,
                     const std::shared_ptr<std::vector<unsigned short> > &indices,
                     Point_2 min, Point_2 max,
                     Eigen::Vector4f uv);


bool add_path(const entt::entity entity, const std::vector<Eigen::Vector2f> &path, const Color color);

bool add_box_data(const entt::entity entity, const AABB_min_max<Point_3> &bounding_box);

bool add_box_data(entt::entity entity,
                  float x_min = -0.5,
                  float y_min = -0.5,
                  float z_min = -0.5,
                  float x_max = 0.5,
                  float y_max = 0.5,
                  float z_max = 0.5);


#endif //HELLO_MAC_GEOMETRY_COMPONENT_H
