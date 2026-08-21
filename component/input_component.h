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
#include "base_geometry/geometry_element/point_2.h"

enum operator_select_status {
    no_select_current = 0,
    select_current    = 1,
};

class Input_Component {
public:
    operator_select_status select_status_ = no_select_current;

    std::vector<std::pair<Combined_shortcut_keys, std::function<wmOperatorStatus (entt::entity,
                              std::chrono::milliseconds ms)> > > shortcut_keys;

    std::function<wmOperatorStatus (const entt::entity entity, const Point_2 temp)> scroll = nullptr;


    Input_Component() = default;

    void add_shortcut_keys(Combined_shortcut_keys temp_w,
                           const std::function<wmOperatorStatus (entt::entity,
                                                                 std::chrono::milliseconds ms)> &function) {
        shortcut_keys.emplace_back(temp_w, function);
    }

    void add_scroll(const std::function<wmOperatorStatus (const entt::entity entity, const Point_2 temp)> function) {
        scroll = function;
    }

    ~Input_Component() = default;

    wmOperatorStatus on_Event(const entt::entity entity, const SDL_Event &event,
                              std::optional<base_event_with_stamp> mouse);
};


#endif //HELLO_MAC_INPUT_COMPONENT_H
