//
// Created by 潘鑫 on 2025/12/23.
//

#ifndef HELLO_MAC_INPUT_COMPONENT_H
#define HELLO_MAC_INPUT_COMPONENT_H


#include <iostream>
#include <utility>
#include <entt/entt.hpp>

#include "base_event.h"
#include "imgui_impl_sdl3.h"
#include "observer_manage.h"

enum operator_select_status {
    no_select_current = 0,
    select_current    = 1,
};

class Input_Component {
public:
    operator_select_status select_status_ = no_select_current;

    Input_Component(
        const std::function<wmOperatorStatus
            (entt::entity, const SDL_Event *)> &function) : on_Event(function) {
    }

    ~Input_Component() {
    }

    std::function<wmOperatorStatus (entt::entity, const SDL_Event *event)> on_Event;
};


#endif //HELLO_MAC_INPUT_COMPONENT_H
