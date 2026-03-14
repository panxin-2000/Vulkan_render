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
    std::queue<std::pair<RND_entity, const std::function<void(void)>> > RND_update_function;

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
        while (!RND_update_function.empty()) {
            auto [vk_data, callback] = RND_update_function.front();
            RND_update_function.pop();
            callback();
            const auto proxy = RND_entt().get_or_emplace<VKR_object_proxy>(vk_data.entity_);
            std::cout << "sdfghjk" << std::endl;
        }
    }

    void render_update_entt(const RND_entity entity, const std::function<void(void)> &callback) {
        std::unique_lock<std::mutex> lock(mtx);
        RND_update_function.emplace(entity, callback);
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
