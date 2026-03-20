//
// Created by 潘鑫 on 2026/1/27.
//

#ifndef HELLO_MAC_VULKAN_RENDER_MANAGE_H
#define HELLO_MAC_VULKAN_RENDER_MANAGE_H
#include <mutex>
#include <thread>
#include <utility>

#include "render_proxy.h"


class vk_render_queue {
private:
    mutable std::mutex mtx;
    std::queue<const std::function<void(void)>> logic_add_function;
    std::queue<const std::function<void(void)>> render_execute_function;
    std::atomic<bool> logic_thread_finished = false;

public:
    static vk_render_queue &instance() {
        static vk_render_queue *instance = nullptr;
        static std::once_flag flag;
        std::call_once(flag, []() {
            instance = new vk_render_queue();
        });
        return *instance;
    }

    void execute_update_lambda() {
        std::unique_lock<std::mutex> lock(mtx);
        if (logic_thread_finished.load() == true) {
            while (!render_execute_function.empty()) {
                auto callback = render_execute_function.front();
                render_execute_function.pop();
                callback();
            }
            logic_thread_finished.store(false);
        }
    }

    void logic_add_finished() {
        // 如果 lambda 正在执行中，那么只有等执行完，那么 render_execute_function 比如为空
        std::unique_lock<std::mutex> lock(mtx);
        if (render_execute_function.empty() == true) {
            std::swap(render_execute_function, logic_add_function);
            logic_thread_finished.store(true);
        } else {
            // 如果 lambda 不在执行中， 那么直接清空
            while (!render_execute_function.empty())
                render_execute_function.pop();
            std::swap(render_execute_function, logic_add_function);
            logic_thread_finished.store(true);
        }
    }

    void render_update_entt(const std::function<void(void)> &callback) {
        std::unique_lock<std::mutex> lock(mtx);
        logic_add_function.emplace(callback);
    }

private:
    vk_render_queue() {
    }

    ~vk_render_queue() {
    }

public:
    vk_render_queue(const vk_render_queue &) = delete;

    vk_render_queue &operator=(const vk_render_queue &) = delete;

    vk_render_queue(vk_render_queue &&) = delete;

    vk_render_queue &operator=(vk_render_queue &&) = delete;
};


#endif //HELLO_MAC_VULKAN_RENDER_MANAGE_H
