//
// Created by 潘鑫 on 2026/5/24.
//

#ifndef HELLO_MAC_PARSE_GEOMETRY_FILE_H
#define HELLO_MAC_PARSE_GEOMETRY_FILE_H

#include <vector>
#include "base_geometry/base.h"
#include "global_singleton.h"

std::optional<AABB_min_max<Point_3> > load_model(const entt::entity entity, const std::string &path);

#endif //HELLO_MAC_PARSE_GEOMETRY_FILE_H
