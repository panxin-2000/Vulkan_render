//
// Created by 潘鑫 on 2025/12/23.
//

#ifndef HELLO_MAC_RENDER_COMPONENT_H
#define HELLO_MAC_RENDER_COMPONENT_H


#include <scene_component.h>
#include "name_component.h"
#include "model_matrix.h"


class model_transform {
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


    static bool check_entity_intersect_point(entt::entity entity, const Point_2 &current_position) {
        if (auto *scene_node = g_entt().try_get<model_transform>(entity)) {
            // 下面这个3d部分是需要去写的，但是只能通过射线来进行检测了
            // if (intersect(scene_node->bounding_box_, current_position)) {
            // return true;
            // }
        }
        return false;
    }
};


#endif //HELLO_MAC_RENDER_COMPONENT_H
