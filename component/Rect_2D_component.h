//
// Created by 潘鑫 on 2026/2/17.
//

#ifndef HELLO_MAC_UI_POSITION_AND_OFFSET_H
#define HELLO_MAC_UI_POSITION_AND_OFFSET_H
#include <scene_component.h>

#include "descriptor_pool.h"
#include "transform_component.h"
#include "name_component.h"
#include "VKR_proxy_component.h"
#include "shader_component.h"
#include "base_geometry/intersect_function.h"


class Rect_2D_transform {
    Point_2 zoom_   = {1, 1};
    Point_2 offset_ = {0, 0};

    AABB_centroid<Point_2> bounding_box_; // 每次都直接计算吧。

public:
    [[nodiscard]] Point_2 get_zoom() const {
        return zoom_;
    }

    [[nodiscard]] Point_2 get_offset() const {
        return offset_;
    }

    Point_2 set_zoom(const Point_2 &zoom) {
        return zoom_ = zoom;
    }

    Point_2 set_offset(const Point_2 &offset) {
        return offset_ = offset;
    }

    Point_2 add_offset(const Point_2 &offset) {
        bounding_box_.add_offset(offset);
        return offset_ = offset + offset_;
    }

    Point_2 multiply_zoom(const Point_2 &zoom) {
        return zoom_ = zoom * zoom_;
    }

    void set_bounding_box(const Point_2 min, const Point_2 max) {
        bounding_box_ = (AABB_min_max<Point_2>(min, max));
    }

    AABB_min_max<Point_2> get_bounding_box() {
        return bounding_box_;
    }
};

bool deal_zoom(const entt::entity entity, const SDL_Event *event);

bool deal_position_offset(const entt::entity entity, const SDL_Event *event);

bool check_entity_intersect_point(const entt::entity entity, const Point_2 &current_position);


bool get_intersect_entity(std::vector<entt::entity> &return_value,
                          const entt::entity entity,
                          const Point_2 &mouse_position);

#endif //HELLO_MAC_UI_POSITION_AND_OFFSET_H
