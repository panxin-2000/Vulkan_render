//
// Created by 潘鑫 on 2026/1/27.
//

#ifndef HELLO_MAC_VK_RENDER_TO_IMAGE_H
#define HELLO_MAC_VK_RENDER_TO_IMAGE_H
#include <atomic>
#include <mutex>
#include <thread>


class vk_render_to_screen {
    mutable std::mutex mtx;
    std::atomic<bool> have_object_need_update = false;
    std::atomic<bool> need_render = true;

public:
    void render_thread() {
        while (need_render) {
            {
                std::unique_lock<std::mutex> lock(mtx);
                init_need_objects(); // 主要是复制内存的操作
                update_need_objects();
            }
            // render_object_function();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            clean_need_objects();
        }
        clean_all_object();
        have_object_need_update = false;
        need_render = true;
    }

    void render_thread_stop() {
        need_render = false;
    }

    static vk_render_to_screen &get_instance() {
        static vk_render_to_screen *instance = nullptr;
        static std::once_flag flag;
        std::call_once(flag, []() {
            instance = new vk_render_to_screen();
        });
        return *instance;
    }

private:
    void init_need_objects() {
        // 内存的搬运
    }


    void clean_all_object() {
        // 正式项目中，确保 vkDeviceWaitIdle 后按顺序销毁资源是专业开发者的标准做法
    }

    void update_need_objects() {
        // 内存内容的更新
        // 先查找放置在哪里来
        // 之后再更新数据
    }

    void clean_need_objects() {
        // 简单的将内存区域标记为没有内容
        // init_need_objects 再根据需要进行移动或者拼接操作
    }

private:
    vk_render_to_screen() {
    }

    ~vk_render_to_screen() {
    }

    vk_render_to_screen(const vk_render_to_screen &) = delete;

    vk_render_to_screen &operator=(const vk_render_to_screen &) = delete;

    vk_render_to_screen(vk_render_to_screen &&) = delete;

    vk_render_to_screen &operator=(vk_render_to_screen &&) = delete;
};


#endif //HELLO_MAC_VK_RENDER_TO_IMAGE_H
