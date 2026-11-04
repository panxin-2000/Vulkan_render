//
// Created by 潘鑫 on 2026/1/13.
//

#ifndef HELLO_MAC_ENTITY_NAME_COMPONENT_H
#define HELLO_MAC_ENTITY_NAME_COMPONENT_H
#include <string>
#include "global_singleton.h"

struct Name_component {
public:
    std::string name_;

    friend std::ostream &operator<<(std::ostream &output,
                                    const Name_component &P) {
        output << P.name_;
        return output;
    }
};

inline std::string get_entity_name(const entt::entity entity) {
    if (const auto name = Logic_entt().try_get<Name_component>(entity)) {
        return name->name_;
    }
    return "";
}


#endif //HELLO_MAC_ENTITY_NAME_COMPONENT_H
