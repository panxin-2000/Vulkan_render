//
// Created by 潘鑫 on 2026/1/27.
//

#ifndef HELLO_MAC_VULKAN_RENDER_MANAGE_H
#define HELLO_MAC_VULKAN_RENDER_MANAGE_H
#include <mutex>
#include <thread>
#include "global_singleton.h"
#include "logic_render_data.h"

class logic_render_data;


class vk_render_queue {
private:
    mutable std::mutex mtx;
    // std::vector<union_render_data> render_objects;
    std::queue<draw_need_vk *> need_init;
    std::queue<draw_need_vk *> need_update;
    std::queue<draw_need_vk *> need_clean;
    // 其实 vector 并不算是很好，用队列的话，更加方便，还能顺便看看怎么做成无锁的队列

public:
    void init_logic_need_resources();

    static vk_render_queue &instance() {
        static vk_render_queue *instance = nullptr;
        static std::once_flag flag;
        std::call_once(flag, []() {
            instance = new vk_render_queue();
        });
        return *instance;
    }


    void clean_vk_render() {
        while (!need_init.empty()) {
            need_init.pop();
        }
        while (!need_update.empty()) {
            need_update.pop();
        }
        while (!need_clean.empty()) {
            need_clean.pop();
        }
    }

    std::optional<draw_need_vk *> get_need_init() {
        std::unique_lock<std::mutex> lock(mtx);
        if (!need_init.empty()) {
            draw_need_vk *val = need_init.front();
            need_init.pop();
            return val;
        }
        return std::nullopt;
    }

    std::optional<draw_need_vk *> get_need_update() {
        std::unique_lock<std::mutex> lock(mtx);
        if (!need_update.empty()) {
            draw_need_vk *val = need_update.front();
            need_update.pop();
            return val;
        }
        return std::nullopt;
    }

    std::optional<draw_need_vk *> get_need_clean() {
        std::unique_lock<std::mutex> lock(mtx);
        if (!need_clean.empty()) {
            draw_need_vk *val = need_clean.front();
            need_clean.pop();
            return val;
        }
        return std::nullopt;
    }


    void render_object_need_init(draw_need_vk *render_object) {
        std::unique_lock<std::mutex> lock(mtx);
        need_init.push(render_object);
        LOG_INFO(g_log(), "add {} to vk_render_queue ", render_object->debug_name);
    }

    void render_object_need_update(draw_need_vk *render_object) {
        std::unique_lock<std::mutex> lock(mtx);
        need_update.push(render_object);
        LOG_INFO(g_log(), "update {} to vk_render_queue ", render_object->debug_name);
    }

    void render_object_need_clean(draw_need_vk *render_object) {
        std::unique_lock<std::mutex> lock(mtx);
        need_clean.push(render_object);
        LOG_INFO(g_log(), "clean {} to vk_render_queue ", render_object->debug_name);
    }

private:
    vk_render_queue() {
    }

    ~vk_render_queue() {
        clean_vk_render();
    }

public:
    vk_render_queue(const vk_render_queue &) = delete;

    vk_render_queue &operator=(const vk_render_queue &) = delete;

    vk_render_queue(vk_render_queue &&) = delete;

    vk_render_queue &operator=(vk_render_queue &&) = delete;
};


#endif //HELLO_MAC_VULKAN_RENDER_MANAGE_H
