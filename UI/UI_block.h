//
// Created by 潘鑫 on 2026/1/17.
//

#ifndef HELLO_MAC_UI_BLOCK_H
#define HELLO_MAC_UI_BLOCK_H


#include "input_component.h"
#include "scene_component.h"
#include "UI_button.h"


class UI_block : public UI_Button {
public:
    UI_block(const std::string &name);

    UI_Button add_button(const std::string &name);
};



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
