//
// Created by 潘鑫 on 2025/12/13.
//

#ifndef HELLO_MAC_FACE_H
#define HELLO_MAC_FACE_H

struct Face {
    half_edge_index bounding_half_edge;

    enum BOUNDARY_TYPE {
        bounding_face,
        hole_face,
    };

    BOUNDARY_TYPE boundary_type;
};

#endif //HELLO_MAC_FACE_H