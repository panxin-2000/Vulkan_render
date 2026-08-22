//
// Created by 潘鑫 on 2026/1/17.
//

#ifndef HELLO_MAC_UI_BLOCK_H
#define HELLO_MAC_UI_BLOCK_H


#include "input_component.h"
#include "scene_component.h"
#include "UI_button.h"

inline entt::entity UI_block(const std::string &name,
                             const float min_x,
                             const float min_y,
                             const float max_x,
                             const float max_y) {
    // const auto entity = UI_button(name, min_x, min_y, max_x, max_y);
    // return entity;
}

/**
 * 在任意一个 UI 上创建一个 button
 * @param entity
 * @param name
 * @param min_x
 * @param min_y
 * @param max_x
 * @param max_y
 * @return 返回创建的button 的 entt::entity
 */
inline entt::entity add_button(const entt::entity entity, const std::string &name,
                               const int min_x,
                               const int min_y,
                               const int max_x,
                               const int max_y) {
    // const auto UI_entity = UI_button(name, min_x, min_y, max_x, max_y);
    // clear_parent_relation(UI_entity);
    // add_relation(entity, UI_entity);
    // return UI_entity;
}


#endif //HELLO_MAC_UI_BLOCK_H
