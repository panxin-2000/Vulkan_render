//
// Created by 潘鑫 on 2026/3/3.
//

#ifndef HELLO_MAC_MESH_COMPONENT_H
#define HELLO_MAC_MESH_COMPONENT_H
#include "vulkan_buffer.h"
#include "global_singleton.h"
#include "render_mesh.h"
#include "shader_common.h"
#include "base_element/point_3.h"

struct share_block {
    std::shared_ptr<void> ptr;
    void *data;
    size_t total_size;
    size_t count;
    size_t single_size;
    // std::vector<VertexAttrib> vertex_attribs;  // 这里暂时清除了
};


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

class Geometry_data : public NonCopyable {
public:
    share_block vertices_;
    share_block indices_;
    std::string mesh_path_;


    Geometry_data() = default;

    ~Geometry_data() = default;


    void set_vertices(const share_block &temp) {
        vertices_ = temp;
    }


    auto get_indices() const {
        return indices_;
    }

    void set_indices(const share_block &indices) {
        indices_ = indices;
    }
};

std::pair<share_block, share_block> load_model(const std::string &path);

void clean_all_mesh_object();


std::vector<VKR_Primitive> create_mesh(const entt::entity entity);

std::vector<VKR_Primitive> get_VKR_mesh(const entt::entity entity);


void add_geometry_data(const entt::entity entity_,
                       const std::shared_ptr<std::vector<Vertex> > &sp_vertices,
                       const std::shared_ptr<std::vector<uint16_t> > &sp_indices);


bool add_geometry_data(entt::entity entity_, const std::string &mesh_path);

bool add_geometry_data(entt::entity entity_,
                       float min_x,
                       float min_y,
                       float max_x,
                       float max_y);

bool add_geometry_data(entt::entity entity_,
                       Point_3 a,
                       Point_3 b,
                       Point_3 c);

#endif //HELLO_MAC_MESH_COMPONENT_H
