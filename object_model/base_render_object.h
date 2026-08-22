//
// Created by 潘鑫 on 2026/8/22.
//

#ifndef HELLO_MAC_BASE_RENDER_OBJECT_H
#define HELLO_MAC_BASE_RENDER_OBJECT_H
#include "global_singleton.h"
#include "shader_resolve.h"

class logic_render_object {
protected:
    const entt::entity entity;

public:
    explicit logic_render_object(const std::string &name);

    logic_render_object &add_shader_path(VKR_shader_paths shader_path);

    void set_random_triangle_color();
};


#endif //HELLO_MAC_BASE_RENDER_OBJECT_H
