//
// Created by 潘鑫 on 2026/3/6.
//

#ifndef HELLO_MAC_3D_MODEL_DISPLAY_H
#define HELLO_MAC_3D_MODEL_DISPLAY_H

#include "global_singleton.h"
#include "base_geometry/base.h"
#include "transform_component.h"
#include "manifold/manifold.h"

entt::entity object_3d_model(const std::string &name, const std::string &mesh_path,
                             const Point_3 offset             = Point_3(0, 0, 0),
                             const Eigen::Quaternionf &rotate = Eigen::Quaternionf::Identity());

entt::entity object_3d_model(const std::string &name, manifold::MeshGL &mesh, const Point_3 offset,
                             const Eigen::Quaternionf &rotate = Eigen::Quaternionf::Identity());


entt::entity add_sky_box(const std::string &name);


#endif //HELLO_MAC_3D_MODEL_DISPLAY_H
