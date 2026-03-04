//
// Created by 潘鑫 on 2026/1/27.
//

#ifndef HELLO_MAC_VULKAN_RENDER_MANAGE_H
#define HELLO_MAC_VULKAN_RENDER_MANAGE_H
#include <mutex>
#include <thread>
#include <utility>


class vk_render_queue {
private:
    mutable std::mutex mtx;
    // std::vector<union_render_data> render_objects;
    std::queue<std::shared_ptr<draw_need_vk> > need_init;
    std::queue<std::shared_ptr<draw_need_vk> > need_update;
    std::queue<std::shared_ptr<draw_need_vk> > need_clean;
    std::queue<std::pair<std::shared_ptr<draw_need_vk>, std::function<void
                             (std::shared_ptr<draw_need_vk> render_object)> > > update_function;
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

    std::optional<std::shared_ptr<draw_need_vk> > get_need_init() {
        std::unique_lock<std::mutex> lock(mtx);
        if (!need_init.empty()) {
            std::shared_ptr<draw_need_vk> val = need_init.front();
            need_init.pop();
            return val;
        }
        return std::nullopt;
    }

    void execute_update_lambda() {
        std::unique_lock<std::mutex> lock(mtx);
        while (!update_function.empty()) {
            auto [vk_data, callback] = update_function.front();
            update_function.pop();
            callback(vk_data);
        }
    }

    std::optional<std::shared_ptr<draw_need_vk> > get_need_update() {
        std::unique_lock<std::mutex> lock(mtx);
        if (!need_update.empty()) {
            std::shared_ptr<draw_need_vk> val = need_update.front();
            need_update.pop();
            return val;
        }
        return std::nullopt;
    }

    std::optional<std::shared_ptr<draw_need_vk> > get_need_clean() {
        std::unique_lock<std::mutex> lock(mtx);
        if (!need_clean.empty()) {
            std::shared_ptr<draw_need_vk> val = need_clean.front();
            need_clean.pop();
            return val;
        }
        return std::nullopt;
    }


    void render_object_need_init(std::shared_ptr<draw_need_vk> render_object) {
        std::unique_lock<std::mutex> lock(mtx);
        need_init.push(render_object);
        LOG_INFO(g_log(), "add {} to vk_render_queue ", render_object->debug_name);
    }

    void render_object_need_update(std::shared_ptr<draw_need_vk> render_object) {
        std::unique_lock<std::mutex> lock(mtx);
        need_update.push(render_object);
        LOG_INFO(g_log(), "update {} to vk_render_queue ", render_object->debug_name);
    }

    // void render_update_descriptor_sets(std::shared_ptr<draw_need_vk> render_object, std::vector<VkDescriptorSet> descriptor_sets) {
    // std::unique_lock<std::mutex> lock(mtx);
    // render_object->vk_descriptor_set = std::move(descriptor_sets);
    // }

    void render_update(std::shared_ptr<draw_need_vk> render_object,
                       const std::function<void(std::shared_ptr<draw_need_vk> render_object)> &callback) {
        std::unique_lock<std::mutex> lock(mtx);
        update_function.emplace(render_object, callback);
    }

    void render_object_need_clean(std::shared_ptr<draw_need_vk> render_object) {
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
