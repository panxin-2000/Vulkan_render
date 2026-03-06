//
// Created by 潘鑫 on 2026/1/16.
//

#ifndef HELLO_MAC_LOGIC_RENDER_DATA_H
#define HELLO_MAC_LOGIC_RENDER_DATA_H

#include <map>

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


#endif //HELLO_MAC_LOGIC_RENDER_DATA_H
