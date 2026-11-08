//
// Created by 潘鑫 on 2026/8/22.
//

#ifndef HELLO_MAC_BASE_RENDER_OBJECT_H
#define HELLO_MAC_BASE_RENDER_OBJECT_H
#include "global_singleton.h"
#include "shader_resolve.h"
#include "shader_component.h"  // 这里感觉有问题

class logic_render_object {
protected:
    const entt::entity entity;

public:
    explicit logic_render_object(const std::string &name);

    [[nodiscard]] entt::entity get_entity() const {
        return entity;
    }
};


#endif //HELLO_MAC_BASE_RENDER_OBJECT_H
