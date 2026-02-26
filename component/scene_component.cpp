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


void clean_render_entity() {
    auto view = g_entt().view<Destroy_tag>(); //得到哪些需要销毁，销毁之后不再显示 // 实体销毁和销毁显示还是需要区分的
    // for (auto it = view.begin(); it != view.end(); ++it)
    // foreach 中 做的优化有点多，先从上一行的 it 来看，它是一个迭代器，会检索需要的类型
    // ++it 不只是++指针，内部还有复杂判读，判断是否包含需要的全部类型，不包括就继续查找，直到到达 end()
    // group 是另一个类似于view的内容，但是呢？会进行内存的搬移，将需要的 entity 移动到 存储的开头部位
    for (const auto it: view) {
        if (const auto render_data = g_entt().try_get<logic_render_data *>(it))
            clean_object_to_render(*render_data);
    }
}
