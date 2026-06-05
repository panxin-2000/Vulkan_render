//
// Created by 潘鑫 on 2025/12/13.
//

#ifndef HELLO_MAC_HALF_EDGE_H
#define HELLO_MAC_HALF_EDGE_H

using Half_edge_index = std::size_t;
using Vertex_index    = std::size_t;
using Face_index      = std::size_t;

struct Half_edge {
    Vertex_index vertex_index;
    Half_edge_index twin_half_edge;
    Half_edge_index next_half_edge;
    Half_edge_index pre_half_edge;
    Face_index incident_face;
};


#endif //HELLO_MAC_HALF_EDGE_H
