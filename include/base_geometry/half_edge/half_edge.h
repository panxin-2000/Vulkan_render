//
// Created by 潘鑫 on 2025/12/13.
//

#ifndef HELLO_MAC_HALF_EDGE_H
#define HELLO_MAC_HALF_EDGE_H


struct Half_edge {
    vertex_index vertex_index;
    half_edge_index twin_half_edge;
    half_edge_index next_half_edge;
    half_edge_index pre_half_edge;
    face_index incident_face;
};



#endif //HELLO_MAC_HALF_EDGE_H