//
// Created by 潘鑫 on 2026/1/27.
//

#ifndef HELLO_MAC_VULKAN_RENDER_MANAGE_H
#define HELLO_MAC_VULKAN_RENDER_MANAGE_H
#include <map>
#include <mutex>
#include <thread>


class logic_render_data;



class vk_render {
private:
    mutable std::mutex mtx;
    // std::vector<union_render_data> render_objects;
    std::vector<logic_render_data *> need_init;
    std::vector<logic_render_data *> need_update;
    std::vector<logic_render_data *> need_clean;
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

    // 不应该是 clear 函数，应该是从其中拿出一个
    void update_need_objects() {
        need_update.clear();
    }

    void init_need_objects() {
        need_clean.clear();
    }

    void clean_need_objects() {
        need_clean.clear();
    }


    void add_render_object_need_init(logic_render_data *render_object) {
        std::unique_lock<std::mutex> lock(mtx);
        need_init.push_back(render_object);
    }

    void add_render_object_need_update(logic_render_data *render_object) {
        std::unique_lock<std::mutex> lock(mtx);
        need_update.push_back(render_object);
    }

    void add_render_object_need_clean(logic_render_data *render_object) {
        std::unique_lock<std::mutex> lock(mtx);
        need_clean.push_back(render_object);
    }

private:
    vk_render() {
    }

    ~vk_render() {
    }

    vk_render(const vk_render &) = delete;

    vk_render &operator=(const vk_render &) = delete;

    vk_render(vk_render &&) = delete;

    vk_render &operator=(vk_render &&) = delete;
};


#endif //HELLO_MAC_VULKAN_RENDER_MANAGE_H
