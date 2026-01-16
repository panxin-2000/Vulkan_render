//
// Created by 潘鑫 on 2025/11/27.
//

#ifndef RENDER_OBJECT_MANAGE_H
#define RENDER_OBJECT_MANAGE_H
#include <vector>
#define GLEW_STATIC
#include <thread>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "render_component.h"


class render_object_manage {
private:
    mutable std::mutex mtx; // 互斥锁（mutable支持const方法加锁）
    std::vector<render_component *> render_objects;
    std::vector<render_component *> need_init;

    std::atomic<bool> have_object_need_update = false;
    std::atomic<bool> need_render = true;

public:
    // 1. 禁用拷贝：防止克隆
    render_object_manage(const render_object_manage &) = delete;

    render_object_manage &operator=(const render_object_manage &) = delete;

    // 2. 禁用移动：防止所有权转移
    render_object_manage(render_object_manage &&) = delete;

    render_object_manage &operator=(render_object_manage &&) = delete;

private:
    render_object_manage() {
        /* 初始化代码 */
        std::cout << "[render_object_manage]" << std::endl;
    }

    ~render_object_manage() {
        /* 释放代码 */
    }

public:
    // static render_object_manage &get_instance() {
    //     static auto *instance = new render_object_manage();
    //     return *instance;
    // }

    // 另一种单例的办法
    static render_object_manage &get_instance() {
        static render_object_manage *instance = nullptr;
        static std::once_flag flag;
        // 线程安全：由系统保证 inside 的 lambda 只执行一次
        std::call_once(flag, []() {
            instance = new render_object_manage();
        });
        return *instance;
    }

    // 添加一个变量，表示渲染的物品中有内容是需要更新的
    // 然后这个变量是多个线程可以修改的，这个线程可以修改，
    // 另一个线程也是可以修改的

    // 然后会发现另外一个问题，那就是帧的问题，比如两个帧或者三个帧，
    // 假设是两个帧，那么更新的时候更新的变量是未渲染的帧，
    // 然后再是渲染的时候，渲染完成之后将该变量重新置位
    /**
     *
     */
    void check_and_update_need_object() {
        if (have_object_need_update == true) {
            for (int i = 0; i < render_objects.size(); ++i) {
                render_objects.at(i)->update_data();
            }
            have_object_need_update = false;
        }
    }

    void check_and_init_need_object() {
    }

    void updata_and_render_object() {
        for (int i = 0; i < render_objects.size(); ++i) {
            render_objects.at(i)->draw();
        }
    }

    void init_need_init_object() {
        for (int i = 0; i < need_init.size(); ++i) {
            need_init.at(i)->init_opengl_start_data();
        }
        need_init.clear();
    }

    void change_update_status(bool status) {
        have_object_need_update = status;
    }

    void add_render_object(render_component *render_object) {
        render_objects.push_back(render_object);
        need_init.push_back(render_object);
    }


    void render_thread(GLFWwindow *window) {
        glfwMakeContextCurrent(window);
        glewExperimental = GL_TRUE;
        glewInit();
        glEnable(GL_DEPTH_TEST);

        while (need_render) {
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); {
                std::unique_lock<std::mutex> lock(mtx);
                init_need_init_object(); // 主要是复制内存的操作
                check_and_init_need_object();
                check_and_update_need_object();
                updata_and_render_object();
            }

            glfwSwapBuffers(window);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        // render_objects.clear();
        // need_init.clear();

        have_object_need_update = false;
        need_render = true;
    }

    void render_thread_stop() {
        std::unique_lock<std::mutex> lock(mtx);
        need_render = false;
        render_objects.clear();
        need_init.clear();
    }


    // 然后这里还需要什么呢？
    // render_objects 需要做什么呢？
    // 将需要的内存数据，传输到GPU
    // 传输到GPU有几个不同的步骤
    //    将GPU上的内存索引 布置好
    //    将数据传输或者更新到相应的GPU内存上
    //    再将GPU内存上的索引删除
    // 再之后是什么呢？
    //    有渲染的步骤，那么就需要有draw的函数
    // 再之后是什么呢？
    //    比如参数配置，这些都是需要在绘制前
};


bool add_object_to_render_manager(render_component *render_object);

bool notify_render_manager_update_objects();

void start_render_manage_thread(GLFWwindow *window);

void end_render_manage_thread();
#endif //RENDER_OBJECT_MANAGE_H
