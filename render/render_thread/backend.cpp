//
// Created by 潘鑫 on 2026/1/29.
//


#include "vk_render_to_image.h"


class vk_render_GPU;

void render_thread_stop() {
    vk_render_GPU::instance().render_thread_stop();
}

void render_thread_stop_and_wait() {
    vk_render_GPU::instance().render_thread_stop_and_wait();
}


void render_thread_start(VK_backend &backend, Engine &engine) {
    std::thread t([&]() {
        vk_render_GPU::instance().render_thread(backend, engine);
    });
    t.detach();
}
