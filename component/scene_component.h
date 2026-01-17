//
// Created by 潘鑫 on 2025/12/25.
//

#ifndef HELLO_MAC_SCENE_COMPONENT_H
#define HELLO_MAC_SCENE_COMPONENT_H
#include <iostream>
#include <vector>
#include <entt/entt.hpp>


class Scene_Component {
private:
    std::vector<entt::entity> parent;
    std::vector<entt::entity> children;

public:
    void add_parent(entt::entity entity) {
        parent.push_back(entity);
    }

    void remove_parent(entt::entity entity) {
        parent.erase(std::remove(parent.begin(), parent.end(), entity), parent.end());
    }

    void remove_children(entt::entity entity) {
        parent.erase(std::remove(parent.begin(), parent.end(), entity), parent.end());
    }

    void add_child(entt::entity entity) {
        children.push_back(entity);
    }
};


class scene_root {
public:
    // 获取全局唯一的注册表引用
    static entt::entity &get() {
        static entt::entity instance = get_entt_instance().create();;
        static std::once_flag flag;

        std::call_once(flag, []() {
            get_entt_instance().emplace<Scene_Component>(instance);
        });
        return instance;
    }

private:
    scene_root() = default; // 禁用构造
};


static entt::entity &get_scene_root() {
    return scene_root::get();
}

/**
 * 将一个节点添加到根节点
 * @param entity 必须存在Scene_Component，如果没有，会在这个函数中添加
 */
inline void scene_root_add_child(entt::entity entity) {
    auto root = get_scene_root();
    auto &parent_scene = get_entt_instance().get<Scene_Component>(root);
    auto &children_scene = get_entt_instance().get<Scene_Component>(entity);
    parent_scene.add_child(entity);
    children_scene.add_child(entity);
}


#endif //HELLO_MAC_SCENE_COMPONENT_H
