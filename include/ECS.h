//
// Created by 潘鑫 on 2026/1/13.
//

#ifndef HELLO_MAC_ECS_H
#define HELLO_MAC_ECS_H
#include <entt/entt.hpp>


class entt_user {
public:
    // 获取全局唯一的注册表引用
    static entt::registry &get() {
        static entt::registry instance;
        return instance;
    }

private:
    entt_user() = default; // 禁用构造
};


static entt::registry &get_entt_instance() {
    return entt_user::get();
}


#endif //HELLO_MAC_ECS_H
