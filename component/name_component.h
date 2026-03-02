//
// Created by 潘鑫 on 2026/1/13.
//

#ifndef HELLO_MAC_ENTITY_NAME_COMPONENT_H
#define HELLO_MAC_ENTITY_NAME_COMPONENT_H
#include <string>

struct Name_component {
    std::string name;
};

inline std::string get_entity_name(const entt::entity entity) {
    if (const auto name = g_entt().try_get<Name_component>(entity)) {
        return name->name;
    }
    return "";
}


#endif //HELLO_MAC_ENTITY_NAME_COMPONENT_H
