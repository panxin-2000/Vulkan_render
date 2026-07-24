//
// Created by 潘鑫 on 2026/1/27.
//

#ifndef HELLO_MAC_VULKAN_RENDER_MANAGE_H
#define HELLO_MAC_VULKAN_RENDER_MANAGE_H

#include <readerwriterqueue.h>


class vk_render_queue {
private:
    moodycamel::BlockingReaderWriterQueue<const std::function<void(void)>> logic_add_function;
    moodycamel::BlockingReaderWriterQueue<const std::function<void(void)>> render_execute_function;
    alignas(64) std::atomic<bool> logic_thread_finished = false;

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
        if (logic_thread_finished.load() == true) {
            std::function<void(void)> callback;
            while (render_execute_function.try_dequeue(callback)) {
                callback();
            }
            logic_thread_finished.store(false);
        }
    }

    void logic_add_finished() {
        // 如果 lambda 正在执行中，那么只有等执行完，那么 render_execute_function 比如为空
        if (render_execute_function.size_approx() == 0) {
            std::swap(render_execute_function, logic_add_function);
            logic_thread_finished.store(true);
        } else {
            // 如果 lambda 不在执行中， 那么将 logic queue 中的内容全部复制到 执行中
            // 那么执行时就是有可能能执行两帧的更新内容了
            std::function<void(void)> callback;
            while (logic_add_function.try_dequeue(callback)) {
                render_execute_function.emplace(callback);
            }
            logic_thread_finished.store(true);
        }
    }

    void render_update_entt(const std::function<void(void)> &callback) {
        logic_add_function.emplace(callback);
    }

    void destroy() {
        std::function<void(void)> callback;
        while (logic_add_function.try_dequeue(callback)) {
        }
        while (render_execute_function.try_dequeue(callback)) {
        }
    };

private:
    vk_render_queue() {
    }

    ~vk_render_queue() = default;

public:
    vk_render_queue(const vk_render_queue &) = delete;

    vk_render_queue &operator=(const vk_render_queue &) = delete;

    vk_render_queue(vk_render_queue &&) = delete;

    vk_render_queue &operator=(vk_render_queue &&) = delete;
};


#endif //HELLO_MAC_VULKAN_RENDER_MANAGE_H
