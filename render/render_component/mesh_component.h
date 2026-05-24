//
// Created by 潘鑫 on 2026/3/3.
//

#ifndef HELLO_MAC_MESH_COMPONENT_H
#define HELLO_MAC_MESH_COMPONENT_H
#include "vulkan_buffer.h"
#include "global_singleton.h"
#include "render_mesh.h"
#include "shader_common.h"
#include "base_geometry/base.h"
#include "Geometry_data.h"


//
// Model_mesh create_mesh_data(const VK_handle &handle, const share_block &vertices,
//                             const share_block &indices_);

struct mesh_and_share {
#ifdef WITH_VULKAN_BACKEND
    VKR_Primitive mesh;
#elif  WITH_OPENGL_BACKEND
    unsigned int buffer;
#endif
    uint16_t shared_number;
};


std::pair<const std::shared_ptr<std::vector<Vertex> >,
          const std::shared_ptr<std::vector<uint16_t> >> load_model(const std::string &path);

void clean_all_mesh_object();

/**
 * 创建一个mesh,所有需要的数据都在 entity 的 Geometry_data 中
 * @param entity
 * @return
 */
std::vector<VKR_Primitive> create_mesh(const entt::entity entity);

std::vector<VKR_Primitive> get_VKR_mesh(const entt::entity entity);




#endif //HELLO_MAC_MESH_COMPONENT_H
