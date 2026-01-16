//
// Created by 潘鑫 on 2025/12/23.
//

#ifndef HELLO_MAC_RENDER_COMPONENT_H
#define HELLO_MAC_RENDER_COMPONENT_H
#include "base_event.h"
#include "ECS.h"
#include "base_element/point_3.h"
#include "shader.h"
#include <entt/entt.hpp>

#include "render_component.h"


class Position_component : public NonCopyable {
public:
    Position_component() {
    }

    // 重写初始化：模拟加载模型
    void initialize() {
    }


    bool set_zoom(const base_event_with_stamp &base_event) {
        zoom.x = zoom.x * std::powf(1.5, base_event.scroll.x * 0.01);
        zoom.y = zoom.y * std::powf(1.5, base_event.scroll.y * 0.01);
    }

    bool set_position_offset(const base_event_with_stamp &base_event) {
        offset = offset + base_event.current_position - base_event.move_position;
        return true;
    }


    bool update_position() {
        auto &storage = get_entt_instance().storage<Position_component>();


        const auto entity = entt::to_entity(storage, *this);
        if (auto *render = get_entt_instance().try_get<render_component>(entity)) {
            Shader_object::data_value_or_ptr data{};
            Shader_object::set_model_transform_zoom_rotate(data.vec_4,
                                                           {zoom.x, zoom.y, 1.0},
                                                           {0.0f, 0.0f, 0.0f}, {offset});
            render->add_uniform("model_transform", Shader_object::gl_mat4, data);
        }
        return true;
    }

    bool update_2D_position_matrix() {
        auto &storage = get_entt_instance().storage<Position_component>();


        const auto entity = entt::to_entity(storage, *this);
        if (auto *render = get_entt_instance().try_get<render_component>(entity)) {
            Shader_object::data_value_or_ptr data{};
            Shader_object::set_model_transform_zoom_rotate(data.vec_4,
                                                           // {0.01f , 0.01f, 1.0},
                                                           {2.0f / get_win_WIDTH(), 2.0f / get_win_HEIGHT(), 1.0},
                                                           {0.0f, 0.0f, 0.0f}, {-1, -1, 0});
            render->add_uniform("model_transform", Shader_object::gl_mat4, data);
        }
        return true;
    }

    Point_2 get_zoom() const {
        return zoom;
    }

    Point_2 get_offset() const {
        return offset;
    }

private:
    Point_2 zoom = {1, 1};
    Point_2 offset = {0, 0};
};

#endif //HELLO_MAC_RENDER_COMPONENT_H
