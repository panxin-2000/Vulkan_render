//
// Created by 潘鑫 on 2026/1/16.
//

#ifndef HELLO_MAC_LOGIC_RENDER_DATA_H
#define HELLO_MAC_LOGIC_RENDER_DATA_H

#include <map>

#include "APP_utility_mixins.h"
#include "mesh_component.h"
#include "utility.h"


/**
 * vertices_changed         <br>
 * indices_changed          <br>
 * texture_path_changed     <br>
 * texture_name_changed     <br>
 * vertex_path_changed      <br>
 * fragment_path_changed    <br>
 * geometry_path_changed    <br>
 * primitive_type_changed   <br>
 * uniform_buffer_changed   <br>
 */
enum status_change : uint16_t {
    no_change              = 0,
    vertices_changed       = 1 << 0,
    indices_changed        = 1 << 1,
    texture_path_changed   = 1 << 2,
    texture_name_changed   = 1 << 3,
    vertex_path_changed    = 1 << 4,
    fragment_path_changed  = 1 << 5,
    geometry_path_changed  = 1 << 6,
    primitive_type_changed = 1 << 7,
    uniform_buffer_changed = 1 << 8,
};

ENABLE_BITWISE_OPERATORS(status_change)


#include "shader_component.h"

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

std::optional<Model_mesh> create_mesh(const entt::entity entity,
                                      std::map<Geometry_data *, mesh_and_share> &map);

inline bool add_geometry_data(entt::entity entity_,
                              float min_x,
                              float min_y,
                              float max_x,
                              float max_y) {
    if (auto *pos = g_entt().try_get<Geometry_data>(entity_)) {
        g_entt().remove<Geometry_data>(entity_);
    }
    g_entt().emplace<Geometry_data>(entity_);

    auto &geometry = g_entt().get<Geometry_data>(entity_);

    /***************设置顶点与索引参数**********************/
    // std::vector<VertexAttrib> vertex_attribs;
    // vertex_attribs.emplace_back(3,GL_FLOAT,GL_FALSE);
    // vertex_attribs.emplace_back(2,GL_FLOAT,GL_FALSE);
    // vertex_attribs.emplace_back(3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) (3 * sizeof(float)));

    struct pos_normal_uv {
        float x, y, z, a, b, c, u, v;
    };

    const auto vertices = std::make_shared<std::vector<pos_normal_uv> >(); //  32  * 4 = 128
    const auto indices  = std::make_shared<std::vector<uint16_t> >();      //  2   * 6 = 12
    // 要改这里，需要改的内容似乎就有点说了，之后再看看怎么改吧。
    {
        indices->push_back(vertices->size() + 0);
        indices->push_back(vertices->size() + 1);
        indices->push_back(vertices->size() + 2);
        indices->push_back(vertices->size() + 2);
        indices->push_back(vertices->size() + 3);
        indices->push_back(vertices->size() + 0);
        vertices->emplace_back(pos_normal_uv{min_x, min_y, 0, 0, 0, 0, 0, 0}); //0 1 2
        vertices->emplace_back(pos_normal_uv{max_x, min_y, 0, 0, 0, 0, 1, 0});
        vertices->emplace_back(pos_normal_uv{max_x, max_y, 0, 0, 0, 0, 1, 1}); // 2 3 0
        vertices->emplace_back(pos_normal_uv{min_x, max_y, 0, 0, 0, 0, 0, 1});
    }
    // 参数这里最重要的是下面的两行

    // 参数这里最重要的是下面的两行
    const share_block vertices_buffer = {
        vertices,
        vertices->data(),
        vertices->size() * sizeof(pos_normal_uv),
        vertices->size(),
        sizeof(pos_normal_uv)
    };
    const share_block indices_buffer = {
        indices,
        indices->data(),
        indices->size() * sizeof(uint16_t),
        indices->size(),
        sizeof(uint16_t)
    };

    geometry.push_vertices(vertices_buffer);
    geometry.set_indices(indices_buffer);
}

#endif //HELLO_MAC_LOGIC_RENDER_DATA_H
