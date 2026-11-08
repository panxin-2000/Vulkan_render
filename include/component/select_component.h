//
// Created by 潘鑫 on 2026/8/16.
//

#ifndef HELLO_MAC_SELECT_COMPONENT_H
#define HELLO_MAC_SELECT_COMPONENT_H


struct select_component {
    bool selected = false;
};

struct load_material : public select_component {
};

#endif //HELLO_MAC_SELECT_COMPONENT_H
