//
// Created by 潘鑫 on 2026/3/16.
//

#ifndef HELLO_MAC_LOAD_GLTF_MODEL_H
#define HELLO_MAC_LOAD_GLTF_MODEL_H

#include "global_singleton.h"
#include "base_geometry/base.h"
#include <Eigen/Eigen>


entt::entity load_gltf_model(const std::string &name, const std::string &mesh_path,
                             const Point_3 offset             = Point_3(0, 0, 0),
                             const Eigen::Quaternionf &rotate = Eigen::Quaternionf::Identity(),
                             const Point_3 zoom               = Point_3(1, 1, 1));


#endif //HELLO_MAC_LOAD_GLTF_MODEL_H
