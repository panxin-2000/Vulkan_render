//
// Created by 潘鑫 on 2025/12/23.
//

#ifndef HELLO_MAC_INPUT_COMPONENT_H
#define HELLO_MAC_INPUT_COMPONENT_H


#include <iostream>
#include <utility>

#include "base_event.h"
#include "base_observer.h"
#include "observer_manage.h"


class Input_Component : public NonCopyable{
public:
    Input_Component(const std::function<bool (entt::entity, base_event_with_stamp)> &function) : on_Event(function) {
    }

    ~Input_Component() {
    }

    const std::function<bool (entt::entity, base_event_with_stamp)> on_Event;
};


#endif //HELLO_MAC_INPUT_COMPONENT_H
