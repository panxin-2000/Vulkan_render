//
// Created by 潘鑫 on 2026/8/22.
//


#include "UI/base_render_object.h"

#include "engine.h"
#include "name_component.h"
#include "../UI/VKR_proxy_component.h"


logic_render_object::logic_render_object(const std::string &name) : entity(Logic_entt().create()) {
    Logic_entt().emplace<Name_component>(entity, name);
}
