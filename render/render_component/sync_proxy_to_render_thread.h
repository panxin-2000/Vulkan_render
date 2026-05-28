//
// Created by 潘鑫 on 2026/3/8.
//

#ifndef HELLO_MAC_SYNC_PROXY_TO_RENDER_THREAD_H
#define HELLO_MAC_SYNC_PROXY_TO_RENDER_THREAD_H
#include "global_singleton.h"


struct UI_transform_dirty {
};

struct Object_transform_dirty {
};

struct Camera_transform_dirty {
};

struct Camera_optical_specifications_dirty {
};

struct add_to_render_tag {
};

struct UI_2D_tag {
};

struct imgui_draw {
};

struct uniform_buffer_update {
};

struct push_constant_update {
};

struct global_uniform_buffer_update {
};

struct descriptor_set_update {
};

struct bindless_set_update_detail {
};

//
void sync_render_data_to_render_thread();

bool clean_VKR_object_proxy(const entt::entity entity);

#endif //HELLO_MAC_SYNC_PROXY_TO_RENDER_THREAD_H
