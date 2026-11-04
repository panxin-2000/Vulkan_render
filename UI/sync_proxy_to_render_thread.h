//
// Created by 潘鑫 on 2026/3/8.
//

#ifndef HELLO_MAC_SYNC_PROXY_TO_RENDER_THREAD_H
#define HELLO_MAC_SYNC_PROXY_TO_RENDER_THREAD_H
#include "global_singleton.h"



//
void sync_render_data_to_render_thread(float time_milliseconds);

bool clean_VKR_object_proxy(const entt::entity entity);

#endif //HELLO_MAC_SYNC_PROXY_TO_RENDER_THREAD_H
