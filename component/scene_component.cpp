//
// Created by 潘鑫 on 2026/2/17.
//
#include "scene_component.h"
#include "UI_component.h"

void scene_root_add_child(entt::entity entity) {
    if (g_entt().all_of<Scene_Component>(entity)) {
        auto root            = get_UI_scene_root();
        auto &parent_scene   = g_entt().get<Scene_Component>(root);
        auto &children_scene = g_entt().get<Scene_Component>(entity);
        parent_scene.add_child_relation(entity);
        children_scene.add_parent_relation(root);
    }
}

void scene_add_child(const entt::entity parent_entity, const entt::entity children_entity) {
    if (g_entt().all_of<Scene_Component>(parent_entity) &&
        g_entt().all_of<Scene_Component>(children_entity)) {
        auto &parent_scene   = g_entt().get<Scene_Component>(parent_entity);
        auto &children_scene = g_entt().get<Scene_Component>(children_entity);

        parent_scene.add_child_relation(children_entity);
        children_scene.add_parent_relation(parent_entity);
    }
}

bool add_relation(const entt::entity parent_entity, const entt::entity children_entity) {
    if (g_entt().all_of<Scene_Component>(parent_entity)) {
        auto &entity_scene = g_entt().get<Scene_Component>(parent_entity);
        entity_scene.add_child_relation(children_entity);
    }
    if (g_entt().all_of<Scene_Component>(children_entity)) {
        auto &entity_scene = g_entt().get<Scene_Component>(children_entity);
        entity_scene.add_parent_relation(parent_entity);
    }
    return true;
}

entt::entity get_parent(const entt::entity entity) {
    if (g_entt().all_of<Scene_Component>(entity)) {
        const auto &entity_scene = g_entt().get<Scene_Component>(entity);
        return entity_scene.get_parent();
    }
    return entt::null;
}

// 只是清理了两个 entity 之间的关系
bool clear_relation(const entt::entity parent_entity, const entt::entity children_entity) {
    if (g_entt().all_of<Scene_Component>(parent_entity)) {
        auto &entity_scene = g_entt().get<Scene_Component>(parent_entity);
        entity_scene.remove_children_relation(children_entity);
    }
    if (g_entt().all_of<Scene_Component>(children_entity)) {
        auto &entity_scene = g_entt().get<Scene_Component>(children_entity);
        entity_scene.remove_parent_ralation();
    }
    return true;
}

bool clear_parent_relation(const entt::entity children_entity) {
    const auto parent_scene = get_parent(children_entity);
    clear_relation(parent_scene, children_entity);
    return true;
}


Scene_Component::~ Scene_Component() {
    const auto &storage = g_entt().storage<Scene_Component>();
    const auto entity   = entt::to_entity(storage, *this);
    clear_relation(parent, entity);


    for (auto it = children.rbegin(); it != children.rend(); ++it)
        if (g_entt().valid(*it)) {
            g_entt().emplace_or_replace<Destroy_tag>(*it);
        }
    // auto children_temp = children;
    // auto parent_temp = parent;
    // for (auto it = children.rbegin(); it != children.rend(); ++it) {
    //     bool clear_parent_relation(const entt::entity children_entity);
    //     clear_parent_relation(*it);
    // }
    // for (auto it = children_temp.rbegin(); it != children_temp.rend(); ++it) {
    //     bool add_relation(const entt::entity parent_entity, const entt::entity children_entity);
    //     add_relation(parent_temp, *it);
    // }
}
