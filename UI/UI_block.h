//
// Created by 潘鑫 on 2026/1/17.
//

#ifndef HELLO_MAC_UI_BLOCK_H
#define HELLO_MAC_UI_BLOCK_H


#include "input_component.h"
#include "name_component.h"
#include "observer_manage.h"
#include "scene_component.h"
#include "UI_button.h"

entt::entity UI_block(const std::string &name,
                      float min_x,
                      float min_y,
                      float max_x,
                      float max_y) {
    auto entity = UI_button(name, min_x, min_y, max_x, max_y);
    // std::cout << "UI_block" << std::endl;
    return entity;
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
entt::entity add_button(entt::entity entity, const std::string &name,
                        int min_x,
                        int min_y,
                        int max_x,
                        int max_y) {
    auto UI_entity = UI_button(name, min_x, min_y, max_x, max_y);
    clear_parent_relation(UI_entity);
    add_relation(entity, UI_entity);
    return UI_entity;
}


#endif //HELLO_MAC_UI_BLOCK_H
