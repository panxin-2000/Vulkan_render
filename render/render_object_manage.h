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
    mutable std::mutex mtx; // 互斥锁（mutable支持const方法加锁）


    class two_data {
    public:
        logic_render_data *logic_data = nullptr;
        Render_thread_data *render_data;

        two_data() {
        }

        ~two_data() {
        }
    };

    std::vector<two_data> render_objects;
    std::vector<logic_render_data *> need_init;

    std::atomic<bool> have_object_need_update = false;
    std::atomic<bool> need_render = true;


    std::map<std::string, shader_and_share> vertex_shader_map_;
    std::map<std::string, shader_and_share> fragment_shader_map_;
    std::map<std::string, shader_and_share> geometry_shader_map_;
    std::map<Vertices_type, buffer_and_share> vertices_map_;
    std::map<Indices_type, buffer_and_share> indices_map_;

public:
    void init_logic_need_resources() {
        for (auto user_render_component: need_init) {
            Shader_object::create_vertex_shader(user_render_component->vertexPath_, &vertex_shader_map_);
            Shader_object::create_fragment_shader(user_render_component->fragmentPath_, &fragment_shader_map_);
            Shader_object::create_geometry_shader(user_render_component->geometryPath_, &geometry_shader_map_);
            create_vertex_buffer(user_render_component->vertices_, &vertices_map_);
            create_element_buffer(user_render_component->indices_, &indices_map_);

            // 内容都创建完成了。之后应该怎么做呢？ 绑定。
        }
    }

    void init_VAO_bind_buffer() {
        for (auto user_render_component: need_init) {
            two_data data;
            data.logic_data = user_render_component;
            data.render_data = new Render_thread_data;
            data.render_data->create_VAO();
            data.render_data->bindVAO();
            bind_vertex_buffer(user_render_component->vertices_, &vertices_map_);
            for (int i = 0; i < user_render_component->vertex_attribs.size(); ++i) {
                glVertexAttribPointer(i, user_render_component->vertex_attribs[i].size,
                                      user_render_component->vertex_attribs[i].type,
                                      user_render_component->vertex_attribs[i].normalized,
                                      user_render_component->vertex_attribs[i].stride,
                                      user_render_component->vertex_attribs[i].pointer);
                glEnableVertexAttribArray(i);
            }

            bind_element_buffer(user_render_component->indices_, &indices_map_);
            data.render_data->unbindVAO();

            data.render_data->size = user_render_component->indices_->size();

            data.render_data->shader_object_.shader_init_and_attach(user_render_component,
                                                                    &vertex_shader_map_,
                                                                    &fragment_shader_map_,
                                                                    &geometry_shader_map_);
            data.render_data->shader_object_.update_uniforms(&user_render_component->uniforms_map);
            render_objects.push_back(data);
        }
    }

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
    void update_render_data() {
        // if (have_object_need_update == true) {
        // for (int i = 0; i < render_objects.size(); ++i) {
        // render_objects.at(i)->update_data();
        // }
        // have_object_need_update = false;
        // }
    }


    void render_object_function() {
        for (int i = 0; i < render_objects.size(); ++i) {
            render_objects.at(i).render_data->draw();
        }
    }

    void init_need_init_object() {
        init_logic_need_resources();
        init_VAO_bind_buffer();
        need_init.clear();
    }

    void change_update_status(bool status) {
        have_object_need_update = status;
    }

    void add_render_object(logic_render_data *render_object) {
        need_init.push_back(render_object);
    }

    GLuint init_sky(void) {
        float skyboxVertices[] = {
            // positions
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,

        };
        GLuint sky_VBO, sky_VAO;
        glGenVertexArrays(1, &sky_VAO);
        glGenBuffers(1, &sky_VBO);
        glBindVertexArray(sky_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, sky_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *) 0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0); // Unbind VAO
        return sky_VAO;
    }

    void render_thread(GLFWwindow *window) {
        glfwMakeContextCurrent(window);
        glewExperimental = GL_TRUE;
        glewInit();
        glEnable(GL_DEPTH_TEST);

        GLuint tem = init_sky();


        while (need_render) {
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); {
                std::unique_lock<std::mutex> lock(mtx);
                init_need_init_object(); // 主要是复制内存的操作
                update_render_data();
                // glBindVertexArray(render_objects.at(0).render_data->VAO);

                // glDrawArrays(GL_TRIANGLES, 0, 3);
                // glBindVertexArray(NULL_GPU_INDEX);


                render_object_function();
            }

            glfwSwapBuffers(window);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
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


bool add_object_to_render_manager(logic_render_data *render_object);

bool notify_render_manager_update_objects();

void start_render_manage_thread(GLFWwindow *window);

void end_render_manage_thread();
#endif //RENDER_OBJECT_MANAGE_H
