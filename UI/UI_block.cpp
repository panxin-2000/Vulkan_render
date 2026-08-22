//
// Created by 潘鑫 on 2026/3/18.
//

#include "UI_block.h"

UI_block::UI_block(const std::string &name) : UI_Button(name) {
}

UI_Button UI_block::add_button(const std::string &name) {
    const UI_Button button(name);
    clear_parent_relation(button.get_entity());
    add_relation(entity, button.get_entity());
    return button;
}
