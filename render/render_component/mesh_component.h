//
// Created by 潘鑫 on 2026/3/3.
//

#ifndef HELLO_MAC_MESH_COMPONENT_H
#define HELLO_MAC_MESH_COMPONENT_H
#include "global_singleton.h"
#include "../render_common/render_mesh.h"
#include "../render_common/Geometry_data.h"


/**
 * 创建一个mesh,所有需要的数据都在 entity 的 Geometry_data 中
 * @param entity
 * @return
 */
Mesh_data get_VKR_mesh(entt::entity entity);

std::vector<VKR_Primitive> create_primitives(entt::entity entity);

std::vector<VKR_Primitive> create_primitives(const Geometry_data &data);

Mesh_data create_mesh_data(const Geometry_data &data);
#endif //HELLO_MAC_MESH_COMPONENT_H
