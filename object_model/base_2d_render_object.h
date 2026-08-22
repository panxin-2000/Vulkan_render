//
// Created by 潘鑫 on 2026/8/22.
//

#ifndef HELLO_MAC_BASE_2D_RENDER_OBJECT_H
#define HELLO_MAC_BASE_2D_RENDER_OBJECT_H


#include "base_render_object.h"

class object_2d : public logic_render_object {
public:
    object_2d(const std::string &name);

    object_2d &add_B_spline_curve();

    object_2d &add_bezier_curve();
};


#endif //HELLO_MAC_BASE_2D_RENDER_OBJECT_H
