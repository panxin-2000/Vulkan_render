//
// Created by 潘鑫 on 2026/1/29.
//

#ifndef HELLO_MAC_BACKEND_H
#define HELLO_MAC_BACKEND_H
#include "vulkan_backend.h"

void render_thread_start(VK_backend &handle);

void render_thread_stop();

void render_thread_stop_and_wait();


#endif //HELLO_MAC_BACKEND_H
