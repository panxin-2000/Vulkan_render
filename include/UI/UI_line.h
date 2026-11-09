//
// Created by 潘鑫 on 2026/4/8.
//

#ifndef HELLO_MAC_UI_LINE_H
#define HELLO_MAC_UI_LINE_H


#include <string>
#include "base_2d_render_object.h"

/**
 * 只存储绘制需要的数据
 */
struct UI_line_data {
    std::vector<Eigen::Vector2f> path;
};

struct Tag_UI_line {
};

struct Tag_UI_line_data_dirty {
};

struct Tag_UI_line_renderable {
};


class UI_line : public object_2d {
public:
    explicit UI_line(const std::string &debug_name,
                     entt::entity parent_entity = entt::null) : object_2d(debug_name, parent_entity) {
        Logic_entt().emplace<UI_line_data>(entity);
        Logic_entt().emplace_or_replace<Tag_UI_line>(entity);
    }

    UI_line &update() {
        Logic_entt().emplace_or_replace<Tag_UI_line_data_dirty>(entity);
        return *this;
    }

    UI_line &add_path(const std::vector<Eigen::Vector2f> &path) {
        Logic_entt().emplace_or_replace<UI_line_data>(entity, UI_line_data(path));
        return *this;
    }


    UI_line &set_renderable(bool renderable) {
        if (renderable == true) {
            Logic_entt().emplace_or_replace<Tag_UI_line_renderable>(entity);
        } else {
            Logic_entt().remove<Tag_UI_line_renderable>(entity);
        }
        return *this;
    }
};


#endif //HELLO_MAC_UI_LINE_H
