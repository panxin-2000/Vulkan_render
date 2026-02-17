//
// Created by 潘鑫 on 2025/12/23.
//

#ifndef HELLO_MAC_RENDER_COMPONENT_H
#define HELLO_MAC_RENDER_COMPONENT_H


#include <Scene_Component.h>
#include "name_component.h"
#include "model_matrix.h"


class UI_positon_and_zoom {
    Point_3 zoom   = {1, 1, 1};
    Point_3 offset = {0, 0, 0};
    Quaternion rotate;
    AABB_centroid<Point_3> bounding_box_; // 每次都直接计算吧。

public:
    [[nodiscard]] Point_3 get_zoom() const {
        return zoom;
    }

    [[nodiscard]] Point_3 get_offset() const {
        return offset;
    }

    void set_bounding_box(const Point_3 min, const Point_3 max) {
        bounding_box_ = AABB_centroid<Point_3>(min, max);
    }
};


#endif //HELLO_MAC_RENDER_COMPONENT_H
