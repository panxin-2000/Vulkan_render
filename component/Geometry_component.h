//
// Created by 潘鑫 on 2026/9/24.
//

#ifndef HELLO_MAC_GEOMETRY_COMPONENT_H
#define HELLO_MAC_GEOMETRY_COMPONENT_H


#include "Geometry_data.h"
#include "global_singleton.h"
#include "PBR_base.h"
#include "base_geometry/base.h"


template<typename vertex_t, typename index_t>
void add_geometry_data(const entt::entity entity,
                       const std::shared_ptr<std::vector<vertex_t> > &sp_vertices,
                       const std::shared_ptr<std::vector<index_t> > &sp_indices) {
    auto &geometry = Logic_entt().get_or_emplace<Geometry_data>(entity);
    geometry.push(sp_vertices, sp_indices);
}

template<typename vertex_t, typename index_t>
void set_geometry_data(const entt::entity entity,
                       const std::shared_ptr<std::vector<vertex_t> > &sp_vertices,
                       const std::shared_ptr<std::vector<index_t> > &sp_indices) {
    auto &geometry = Logic_entt().emplace_or_replace<Geometry_data>(entity);
    geometry.push(sp_vertices, sp_indices);
}

void clean_geometry_data(entt::entity entity);

bool add_2D_bound_box_geometry(entt::entity entity,
                               Eigen::Vector2f min,
                               Eigen::Vector2f max);

bool add_2D_bound_box_geometry(entt::entity entity,
                               Eigen::Vector3f min,
                               Eigen::Vector3f max);

bool add_round_box_geometry(entt::entity entity,
                            Eigen::Vector3f min,
                            Eigen::Vector3f max);

bool add_triangle_geometry(entt::entity entity,
                           Eigen::Vector3f a,
                           Eigen::Vector3f b,
                           Eigen::Vector3f c);


bool add_path(entt::entity entity, const std::vector<Eigen::Vector2f> &path, Color color);

bool add_box_data(entt::entity entity, const AABB_min_max<Eigen::Vector3f> &bounding_box);

bool add_box_data(entt::entity entity,
                  float x_min = -0.5,
                  float y_min = -0.5,
                  float z_min = -0.5,
                  float x_max = 0.5,
                  float y_max = 0.5,
                  float z_max = 0.5);


#endif //HELLO_MAC_GEOMETRY_COMPONENT_H
