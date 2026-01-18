//
// Created by 潘鑫 on 2026/1/17.
//

#ifndef HELLO_MAC_UI_BLOCK_H
#define HELLO_MAC_UI_BLOCK_H


#include "input_component.h"
#include "ECS.h"
#include "entity_name_component.h"
#include "observer_manage.h"
#include "scene_component.h"
#include "UI_button.h"

class UI_block : public UI_button {
private:
    entt::entity entity_;

public:
    UI_block(const std::string &name,
             float min_x,
             float min_y,
             float max_x,
             float max_y) : UI_button(name, min_x, min_y, max_x, max_y) {
        std::cout << "UI_block" << std::endl;
        // set_Input_Component_on_Event_function()
        scene_root_add_child(entity_);
    }

    UI_button *add_button(const std::string &name,
                          int min_x,
                          int min_y,
                          int max_x,
                          int max_y) {
        auto UI_entity = new UI_button(name, min_x, min_y, max_x, max_y);


        auto &parent_scene = get_entt_instance().get<Scene_Component>(entity_);
        auto &children_scene = get_entt_instance().get<Scene_Component>(UI_entity->get_entity());
        parent_scene.add_child(UI_entity->get_entity());
        children_scene.add_parent(entity_);
        return UI_entity;
    }
};


#endif //HELLO_MAC_UI_BLOCK_H
