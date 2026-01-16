//
// Created by 潘鑫 on 2025/2/26.
//

#ifndef CUBE_H
#define CUBE_H
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include  "APP_utility_mixins.h"

#define GLEW_STATIC
#include <GL/glew.h>

#include "buffer_object.h"
#include "EBO_object.h"
#include "VBO_object.h"
#include "texture_TBO.h"
#include "shader.h"


#include "shader_common.h"


class Render_thread_data : public NonCopyable {
private:
    vertex_array_VAO *VAO_new = nullptr;
    std::vector<Texture_TBO> TBO;

public:
    Shader_object shader_object_ = {};
    int size;

    Render_thread_data() {
    }

    Render_thread_data(const Render_thread_data &) = delete;

    // 2. 禁止拷贝赋值运算符
    Render_thread_data &operator=(const Render_thread_data &) = delete;


    bool add_texture_path(char const *path, char const *texture_name) {
        Texture_TBO texture;
        texture.set_path(path, texture_name);
        TBO.push_back(texture);
    }

    void render_object_shader_init();

    ~Render_thread_data() {
        // delete shader_object_;
    }

    void init_and_bind_VAO() {
        if (VAO_new == nullptr) {
            VAO_new = new vertex_array_VAO;
            VAO_new->bind();
        } else {
            VAO_new->bind();
        }
    }

    void draw() {
        shader_object_.use_shader_program();

        init_and_bind_VAO();

        for (int i = 0; i < TBO.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            TBO.at(i).bind(); // 为什么会有多个区域呢？ 我记得好像是因为可能不同的属性会放置在不同的缓冲区
            glUniform1i(glGetUniformLocation(shader_object_.get_shader_program(),
                                             TBO.at(i).get_texture_name()),
                        i);
        }
        // if (EBO_object.EBO_new != nullptr) {
        glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, (void *) 0);
        // } else {
        // glDrawArrays(GL_TRIANGLES, 0, VBO_object.get_draw_count()); //count还是需要去获取的
        // }
        glBindVertexArray(NULL_GPU_INDEX);
    }
};


#endif //CUBE_H
