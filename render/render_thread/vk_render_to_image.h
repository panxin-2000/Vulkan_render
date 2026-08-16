//
// Created by 潘鑫 on 2026/1/27.
//

#ifndef HELLO_MAC_VK_RENDER_TO_IMAGE_H
#define HELLO_MAC_VK_RENDER_TO_IMAGE_H
#include <atomic>
#include <mutex>
#include <thread>
#include "engine.h"
#include "vulkan_backend.h"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE


class vk_render_GPU {
    mutable std::mutex mtx;
    std::atomic<bool> have_object_need_update = false;
#define not_start 0
#define running 1
#define need_stop 2
    std::atomic<uint32_t> need_render = not_start; // 这里状态有点少了，需要 未开始，运行中，需停止


public:
    void render_once(VK_backend &backend, Engine &engine);

    void exit_and_clean(VK_backend &backend);

    void render_thread(VK_backend &backend, Engine &engine);

    // 显式同步：即使使用 detach，也应通过原子变量（如 std::atomic<bool>）或信号量
    // 通知子线程退出，并确保其在主线程销毁全局资源前完成清理
    void render_thread_stop();

    void render_thread_stop_and_wait();

    static vk_render_GPU &instance();

private:
    static void clean_discard_vulkan_handle(uint64_t finished_timeline);

    vk_render_GPU() {
    }

    ~vk_render_GPU() = default;

public:
    vk_render_GPU(const vk_render_GPU &) = delete;

    vk_render_GPU &operator=(const vk_render_GPU &) = delete;

    vk_render_GPU(vk_render_GPU &&) = delete;

    vk_render_GPU &operator=(vk_render_GPU &&) = delete;
};


#endif //HELLO_MAC_VK_RENDER_TO_IMAGE_H
