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

#include "VBO_object.h"
#include "texture_TBO.h"
#include "shader.h"


#include "shader_common.h"


class Render_thread_data : public NonCopyable {
private:
    std::vector<Texture_TBO> TBO;

public:
    GLuint VAO;
    Shader_object shader_object_ = {};
    int size;

    Render_thread_data() {
    }

    bool create_VAO() {
        glGenVertexArrays(1, &VAO);
    }

    bool bindVAO() {
        glBindVertexArray(VAO);
    }

    bool unbindVAO() {
        glBindVertexArray(NULL_GPU_INDEX);
    }


    ~Render_thread_data() {
    }

    bool add_texture_path(char const *path, char const *texture_name) {
        Texture_TBO texture;
        texture.set_path(path, texture_name);
        TBO.push_back(texture);
    }


    void draw() {
        shader_object_.use_shader_program();

        bindVAO();

        glBindVertexArray(VAO);

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
        unbindVAO();
    }
};

class union_render_data {
public:
    logic_render_data *logic_data = nullptr;
    Render_thread_data *render_data;

    union_render_data() {
    }

    ~union_render_data() {
    }
};

#endif //CUBE_H
