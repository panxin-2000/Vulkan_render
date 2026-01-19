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

#include "shader_common.h"
#include "Render_thread_data.h"
#include "logic_render_data.h"
#include "VBO_object.h"

class render_object_manage {
private:
    mutable std::mutex mtx;
    std::vector<union_render_data> render_objects;
    std::vector<logic_render_data *> need_init;
    std::vector<logic_render_data *> need_update;
    std::vector<logic_render_data *> need_clean;

    std::atomic<bool> have_object_need_update = false;
    std::atomic<bool> need_render = true;

    std::map<std::string, shader_and_share> vertex_shader_map_;
    std::map<std::string, shader_and_share> fragment_shader_map_;
    std::map<std::string, shader_and_share> geometry_shader_map_;
    std::map<Vertices_type, buffer_and_share> vertices_map_;
    std::map<Indices_type, buffer_and_share> indices_map_;

public:
    void init_logic_need_resources();

    void init_VAO_bind_buffer();

public:
    static render_object_manage &get_instance();

    // 添加一个变量，表示渲染的物品中有内容是需要更新的
    // 然后这个变量是多个线程可以修改的，这个线程可以修改，
    // 另一个线程也是可以修改的

    // 然后会发现另外一个问题，那就是帧的问题，比如两个帧或者三个帧，
    // 假设是两个帧，那么更新的时候更新的变量是未渲染的帧，
    // 然后再是渲染的时候，渲染完成之后将该变量重新置位

    void render_object_function() {
        for (int i = 0; i < render_objects.size(); ++i) {
            // std::cout << render_objects.at(i).logic_data->debug_name << std::endl;
            render_objects.at(i).render_data->draw();
        }
    }

    void init_need_objects() {
        init_logic_need_resources();
        init_VAO_bind_buffer();
        need_init.clear();
    }

    void update_need_objects() {
        for (auto user_render_component: need_update) {
            if (user_render_component->get_status_change() & vertices_changed) {
            }
            if (user_render_component->get_status_change() & indices_changed) {
            }
            if (user_render_component->get_status_change() & texture_path_changed) {
            }
            if (user_render_component->get_status_change() & texture_name_changed) {
            }
            if (user_render_component->get_status_change() & vertex_path_changed) {
            }
            if (user_render_component->get_status_change() & fragment_path_changed) {
            }
            if (user_render_component->get_status_change() & geometry_path_changed) {
            }
            if (user_render_component->get_status_change() & primitive_type_changed) {
            }
            if (user_render_component->get_status_change() & uniform_buffer_changed) {
                for (const auto &render_object: render_objects) {
                    if (render_object.logic_data == user_render_component)
                        render_object.render_data->shader_object_.update_uniforms(&user_render_component->uniforms_map);
                }
            }
        }
        need_update.clear();
    }

    void clean_need_objects() {
        for (auto render_component: need_clean) {
            std::cout << " clean_need_objects" << std::endl;
        }
        need_clean.clear();
    }


    void add_render_object_need_init(logic_render_data *render_object);

    void add_render_object_need_update(logic_render_data *render_object);

    void add_render_object_need_clean(logic_render_data *render_object);


    void render_thread(GLFWwindow *window) {
        glfwMakeContextCurrent(window);
        glewExperimental = GL_TRUE;
        glewInit();
        glDisable(GL_DEPTH_TEST);

        while (need_render) {
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); {
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    init_need_objects(); // 主要是复制内存的操作
                    update_need_objects();
                }
                render_object_function();
            }
            glfwSwapBuffers(window);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            clean_need_objects();
        }
        have_object_need_update = false;
        need_render = true;
    }

    void render_thread_stop() {
        std::unique_lock<std::mutex> lock(mtx);
        need_render = false;
        render_objects.clear();
        need_init.clear();
    }

private:
    render_object_manage() {
    }

    ~render_object_manage() {
    }

    // 1. 禁用拷贝：防止克隆
    render_object_manage(const render_object_manage &) = delete;

    render_object_manage &operator=(const render_object_manage &) = delete;

    // 2. 禁用移动：防止所有权转移
    render_object_manage(render_object_manage &&) = delete;

    render_object_manage &operator=(render_object_manage &&) = delete;

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

#endif //RENDER_OBJECT_MANAGE_H
