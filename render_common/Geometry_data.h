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

Model_mesh create_mesh(Geometry_data &data,
                       std::map<Geometry_data *, mesh_and_share> &map);


#endif //HELLO_MAC_LOGIC_RENDER_DATA_H
