//
// Created by 潘鑫 on 2025/12/25.
//

#ifndef HELLO_MAC_SCENE_COMPONENT_H
#define HELLO_MAC_SCENE_COMPONENT_H
#include <iostream>
#include <vector>
#include <entt/entt.hpp>
#include "base_event.h"
#include "base_element/point_3.h"
#include "base_element/intersect/objects_intersect_with_point.h"
#include "global_singleton.h"
#include "backend.h"
#include "mesh_component.h"


class Scene_Component {
public:
    entt::entity parent = entt::null;
    std::vector<entt::entity> children;

public:
    ~Scene_Component();

    void add_parent_relation(entt::entity entity) {
        parent = entity;
    }

    [[nodiscard]] entt::entity get_parent() const {
        return parent;
    }

    void remove_parent_relation() {
        parent = entt::null;
    }

    void remove_children_relation(const entt::entity entity) {
        children.erase(std::remove(children.begin(), children.end(), entity), children.end());
    }

    void add_child_relation(const entt::entity entity) {
        // 这里需要想办法去除一下重复的内容
        children.push_back(entity);
    }
};


/**
 * 将一个节点添加到根节点
 * @param entity 必须存在Scene_Component，如果没有，会在这个函数中添加
 */

void scene_add_child(const entt::entity parent_entity, const entt::entity children_entity);


entt::entity get_parent(const entt::entity entity);


bool clear_parent_relation(const entt::entity children_entity);

// 最主要使用的函数应该是下面四个个，添加联系与删除联系
void scene_root_add_child(entt::entity entity);

void world_root_add_child(entt::entity entity);

bool add_relation(const entt::entity parent_entity, const entt::entity children_entity);

bool clear_relation(const entt::entity parent_entity, const entt::entity children_entity);


void clean_render_entity();
#endif //HELLO_MAC_SCENE_COMPONENT_H
