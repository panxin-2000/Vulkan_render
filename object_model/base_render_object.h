//
// Created by 潘鑫 on 2026/8/22.
//

#ifndef HELLO_MAC_BASE_RENDER_OBJECT_H
#define HELLO_MAC_BASE_RENDER_OBJECT_H
#include "global_singleton.h"
#include "shader_resolve.h"
#include "shader_component.h"

class logic_render_object {
protected:
    const entt::entity entity;

public:
    explicit logic_render_object(const std::string &name);

    logic_render_object &add_shader_path(VKR_shader_paths shader_path);

    bool set_random_triangle_color();

    entt::entity get_entity() const {
        return entity;
    }

    template<typename T1>
    logic_render_object &add_render_parameter(const std::string &binding_name, T1 &binding_data) {
        set_render_parameter(entity, binding_name, binding_data);
        return *this;
    }

    template<typename T1>
    logic_render_object &add_push_constant_parameter(const std::string &binding_name, T1 &binding_data) {
        set_push_constant_parameter(entity, binding_name, binding_data);
        return *this;
    }
};


#endif //HELLO_MAC_BASE_RENDER_OBJECT_H
