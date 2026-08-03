//
// Created by 潘鑫 on 2026/8/3.
//

#ifndef HELLO_MAC_NANO_VDB_MODEL_H
#define HELLO_MAC_NANO_VDB_MODEL_H
#include "global_singleton.h"
#include "base_geometry/base.h"

entt::entity add_volume_pass(const std::string &name,
                             const Point_3 offset             = Point_3(500, 200, 0),
                             const Eigen::Quaternionf &rotate = Eigen::Quaternionf::Identity());


#endif //HELLO_MAC_NANO_VDB_MODEL_H
