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

bool add_object_to_render(logic_render_data *render_object);

bool update_object_to_render(logic_render_data *render_object);

bool clean_object_to_render(logic_render_data *render_object);

void start_render_manage_thread(GLFWwindow *window);

void end_render_manage_thread();

struct PendingDestroyTag {
};

#endif //HELLO_MAC_GLOBAL_SINGLETON_H
