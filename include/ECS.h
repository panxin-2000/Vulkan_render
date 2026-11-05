//
// Created by 潘鑫 on 2026/1/13.
//

#ifndef HELLO_MAC_ECS_H
#define HELLO_MAC_ECS_H
#include <entt/entt.hpp>


class entt_logic_thread {
public:
    // 获取全局唯一的注册表引用
    static entt::registry &get() {
        static entt::registry instance;
        return instance;
    }

private:
    entt_logic_thread() = default; // 禁用构造
};


class entt_render_thread {
public:
    // 获取全局唯一的注册表引用
    static entt::registry &get() {
        static entt::registry instance;
        return instance;
    }

private:
    entt_render_thread() = default; // 禁用构造
};


#endif //HELLO_MAC_ECS_H
