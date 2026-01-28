//
// Created by 潘鑫 on 2026/1/27.
//

#ifndef HELLO_MAC_VULKAN_RENDER_MANAGE_H
#define HELLO_MAC_VULKAN_RENDER_MANAGE_H
#include <map>
#include <mutex>
#include <thread>


class logic_render_data;

class vk_render_manage {
private:
    mutable std::mutex mtx;
    // std::vector<union_render_data> render_objects;
    std::vector<logic_render_data *> need_init;
    std::vector<logic_render_data *> need_update;
    std::vector<logic_render_data *> need_clean;
    // 其实 vector 并不算是很好，用队列的话，更加方便，还能顺便看看怎么做成无锁的队列


    // std::map<std::string, shader_and_share> vertex_shader_map_;
    // std::map<std::string, shader_and_share> fragment_shader_map_;
    // std::map<std::string, shader_and_share> geometry_shader_map_;
    // std::map<std::string, texture_and_share> texture_map_;
    // std::map<Vertices_type, buffer_and_share> vertices_map_;
    // std::map<Indices_type, buffer_and_share> indices_map_;

public:
    void init_logic_need_resources();

    static vk_render_manage &get_instance() {
        static vk_render_manage *instance = nullptr;
        static std::once_flag flag;
        std::call_once(flag, []() {
            instance = new vk_render_manage();
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
    vk_render_manage() {
    }

    ~vk_render_manage() {
    }

    vk_render_manage(const vk_render_manage &) = delete;

    vk_render_manage &operator=(const vk_render_manage &) = delete;

    vk_render_manage(vk_render_manage &&) = delete;

    vk_render_manage &operator=(vk_render_manage &&) = delete;
};


#endif //HELLO_MAC_VULKAN_RENDER_MANAGE_H
