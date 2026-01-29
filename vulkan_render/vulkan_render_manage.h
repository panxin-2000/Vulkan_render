//
// Created by 潘鑫 on 2026/1/27.
//

#ifndef HELLO_MAC_VULKAN_RENDER_MANAGE_H
#define HELLO_MAC_VULKAN_RENDER_MANAGE_H
#include <mutex>
#include <thread>


class logic_render_data;


class vk_render {
private:
    mutable std::mutex mtx;
    // std::vector<union_render_data> render_objects;
    std::queue<logic_render_data *> need_init;
    std::queue<logic_render_data *> need_update;
    std::queue<logic_render_data *> need_clean;
    // 其实 vector 并不算是很好，用队列的话，更加方便，还能顺便看看怎么做成无锁的队列

public:
    void init_logic_need_resources();

    static vk_render &instance() {
        static vk_render *instance = nullptr;
        static std::once_flag flag;
        std::call_once(flag, []() {
            instance = new vk_render();
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

    std::optional<logic_render_data *> get_need_init() {
        std::unique_lock<std::mutex> lock(mtx);
        if (!need_init.empty()) {
            logic_render_data *val = need_init.front();
            need_init.pop();
            return val;
        }
        return std::nullopt;
    }

    std::optional<logic_render_data *> get_need_update() {
        std::unique_lock<std::mutex> lock(mtx);
        if (!need_init.empty()) {
            logic_render_data *val = need_init.front();
            need_init.pop();
            return val;
        }
        return std::nullopt;
    }

    std::optional<logic_render_data *> get_need_clean() {
        std::unique_lock<std::mutex> lock(mtx);
        if (!need_init.empty()) {
            logic_render_data *val = need_init.front();
            need_init.pop();
            return val;
        }
        return std::nullopt;
    }


    void render_object_need_init(logic_render_data *render_object) {
        std::unique_lock<std::mutex> lock(mtx);
        need_init.push(render_object);
    }

    void render_object_need_update(logic_render_data *render_object) {
        std::unique_lock<std::mutex> lock(mtx);
        need_update.push(render_object);
    }

    void render_object_need_clean(logic_render_data *render_object) {
        std::unique_lock<std::mutex> lock(mtx);
        need_clean.push(render_object);
    }

private:
    vk_render() {
    }

    ~vk_render() {
        clean_vk_render();
    }

public:
    vk_render(const vk_render &) = delete;

    vk_render &operator=(const vk_render &) = delete;

    vk_render(vk_render &&) = delete;

    vk_render &operator=(vk_render &&) = delete;
};


#endif //HELLO_MAC_VULKAN_RENDER_MANAGE_H
