//
// Created by 潘鑫 on 2026/1/13.
//

#ifndef HELLO_MAC_ECS_H
#define HELLO_MAC_ECS_H
#include <entt/entt.hpp>


static inline entt::registry &get_entt_instance() {
    static auto *instance = new entt::registry;
    return *instance;
}


#endif //HELLO_MAC_ECS_H
