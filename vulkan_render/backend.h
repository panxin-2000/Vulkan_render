//
// Created by 潘鑫 on 2026/1/29.
//

#ifndef HELLO_MAC_BACKEND_H
#define HELLO_MAC_BACKEND_H

#include "logic_render_data.h"
#include "vulkan_device_handle.h"
#include "vulkan_render_manage.h"


void render_thread_start(VKDevice &handle);

void render_thread_stop();

void render_thread_stop_and_wait();

inline bool add_object_to_render(logic_render_data *render_object) {
    vk_render_queue::instance().render_object_need_init(render_object);
    return true;
}

inline bool update_object_to_render(logic_render_data *render_object) {
    vk_render_queue::instance().render_object_need_init(render_object);
    return true;
}

inline bool clean_object_to_render(logic_render_data *render_object) {
    vk_render_queue::instance().render_object_need_init(render_object);
    return true;
}

#endif //HELLO_MAC_BACKEND_H
