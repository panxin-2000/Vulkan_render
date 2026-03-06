//
// Created by 潘鑫 on 2026/3/3.
//

#ifndef HELLO_MAC_MESH_COMPONENT_H
#define HELLO_MAC_MESH_COMPONENT_H
#include "vulkan_buffer.h"
#include "global_singleton.h"
#include "render_mesh.h"

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
    Model_mesh mesh;
#elif  WITH_OPENGL_BACKEND
    unsigned int buffer;
#endif
    uint16_t shared_number;
};

class Geometry_data : public NonCopyable {
public:
    std::string mesh_path_;

    std::vector<share_block> vertices_vector;
    share_block indices_;


    Geometry_data() = default;

    ~Geometry_data() = default;


    void push_vertices(const share_block &temp) {
        vertices_vector.push_back(temp);
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


std::optional<Model_mesh> create_mesh(const entt::entity entity);


bool add_geometry_data(entt::entity entity_,
                       float min_x,
                       float min_y,
                       float max_x,
                       float max_y);

#endif //HELLO_MAC_MESH_COMPONENT_H
