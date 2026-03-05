//
// Created by 潘鑫 on 2026/1/20.
//

#ifndef HELLO_MAC_GLOBAL_SINGLETON_H
#define HELLO_MAC_GLOBAL_SINGLETON_H

#include "quill/LogMacros.h"
#include "quill/Logger.h"
#include <entt/entt.hpp>

#include "quill_log.h"
#include "ECS.h"

class quill_log;

static quill::Logger *g_log() {
    return quill_log::get();
}

class entt_user;

static entt::registry &g_entt() {
    return entt_user::get();
}


static inline int get_win_WIDTH() {
    return 800;
}

static inline int get_win_HEIGHT() {
    return 600;
}

class logic_render_object;


struct Destroy_tag {
};

struct Position_update_tag {
};

struct need_render_tag {
};


#endif //HELLO_MAC_GLOBAL_SINGLETON_H
