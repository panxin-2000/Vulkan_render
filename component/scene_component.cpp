//
// Created by 潘鑫 on 2026/2/17.
//
#include "scene_component.h"

#include "bounding_box_2d.h"
#include "transform_component.h"
#include "Rect_2D_component.h"
#include "UI_manager.h"
#include "world_scene_root.h"

void world_root_add_child(const entt::entity entity) {
    const auto root = get_world_root();
    add_relation(root, entity);
}

void UI_root_add_child(const entt::entity entity) {
    const auto root = get_UI_scene_root();
    add_relation(root, entity);
}


bool add_relation(const entt::entity parent_entity, const entt::entity children_entity) {
    assert(Logic_entt().all_of<Scene_Component>(parent_entity));
    auto &parent_entity_scene = Logic_entt().get<Scene_Component>(parent_entity);
    parent_entity_scene.add_child_entity(children_entity);
    auto &children_entity_scene = Logic_entt().get_or_emplace<Scene_Component>(children_entity);
    children_entity_scene.set_parent_entity(parent_entity);
    return true;
}

entt::entity get_parent(const entt::entity entity) {
    if (Logic_entt().all_of<Scene_Component>(entity)) {
        const auto &entity_scene = Logic_entt().get<Scene_Component>(entity);
        return entity_scene.get_parent();
    }
    return entt::null;
}

std::vector<entt::entity> get_children(const entt::entity entity) {
    if (const auto temp = Logic_entt().try_get<Scene_Component>(entity)) {
        return temp->get_children();
    }
    return {};
}

// 只是清理了两个 entity 之间的关系
bool clear_relation(const entt::entity parent_entity, const entt::entity children_entity) {
    if (Logic_entt().all_of<Scene_Component>(parent_entity)) {
        auto &entity_scene = Logic_entt().get<Scene_Component>(parent_entity);
        entity_scene.remove_child_entity(children_entity);
    }
    if (Logic_entt().all_of<Scene_Component>(children_entity)) {
        auto &entity_scene = Logic_entt().get<Scene_Component>(children_entity);
        entity_scene.clean_parent_entity();
    }
    return true;
}

bool clear_parent_relation(const entt::entity children_entity) {
    const auto parent_scene = get_parent(children_entity);
    if (parent_scene != entt::null) {
        clear_relation(parent_scene, children_entity);
    }
    return true;
}

void Scene_Component::clean_parent_relation() {
    const auto &storage = Logic_entt().storage<Scene_Component>();
    const auto entity   = entt::to_entity(storage, *this);
    clear_relation(parent_, entity);
    clean_parent_entity();
}

Scene_Component::~ Scene_Component() {
    // 这里我不确定是否有问题
    // 先执行复制，再在旧的位置调用清理函数
    // 新的上的关系没有改变
    // 从旧的位置上全部复制就没有问题，否则就有问题

    clean_parent_entity();
    clean_children_entity();
}


void clean_render_entity() {
    auto view = Logic_entt().view<Logic_destroy_tag>(); //得到哪些需要销毁，销毁之后不再显示 // 实体销毁和销毁显示还是需要区分的
    // for (auto it = view.begin(); it != view.end(); ++it)
    // foreach 中 做的优化有点多，先从上一行的 it 来看，它是一个迭代器，会检索需要的类型
    // ++it 不只是++指针，内部还有复杂判读，判断是否包含需要的全部类型，不包括就继续查找，直到到达 end()
    // group 是另一个类似于view的内容，但是呢？会进行内存的搬移，将需要的 entity 移动到 存储的开头部位
    for (const auto entity: view) {
        if (Logic_entt().all_of<Scene_Component>(entity)) {
            clear_parent_relation(entity);
        }
        clean_VKR_object_proxy(entity);
    }
    Logic_entt().destroy(view.begin(), view.end()); // 执行销毁程序
}


bool check_entity_intersect_point(const entt::entity entity, const Eigen::Vector2f &current_position) {
    if (auto *scene_node = Logic_entt().try_get<Bounding_Box_2D>(entity)) {
        AABB_min_max<Point_2> temp{{scene_node->box[0], scene_node->box[1]}, {scene_node->box[2], scene_node->box[3]}};
        if (is_intersect(temp, {current_position.x(), current_position.y()})) {
            return true;
        }
    }
    return false;
}


bool get_intersect_entity(std::vector<entt::entity> &return_value,
                          const entt::entity entity,
                          const Eigen::Vector2f &mouse_position) {
    if (const auto *scene_node = Logic_entt().try_get<Scene_Component>(entity)) {
        for (const entt::entity children_entity: scene_node->children_) {
            if (check_entity_intersect_point(children_entity, mouse_position)) {
                return_value.push_back(children_entity);
                get_intersect_entity(return_value, children_entity, mouse_position);
            }
        }
    }
    return false;
}
