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
#include "Texture_logic.h"
#include "shader.h"


#include "shader_common.h"


class Render_thread_data : public NonCopyable {
private:

public:
    GLuint VAO;
    std::vector<Texture_TBO> TBO_s_;
    Shader_object shader_object_ = {};
    int size_;
    bool have_indices_ = false;

    int get_draw_size() const {
        return size_;
    }

    void set_have_indices_(bool have_indices) {
        have_indices_ = have_indices;
    }

    bool get_have_indices() const {
        return have_indices_;
    }

    void set_draw_size(const int size) {
        size_ = size;
    }

    Render_thread_data() {
    }

    bool create_VAO() {
        glGenVertexArrays(1, &VAO);
    }

    bool bind_VAO() const {
        glBindVertexArray(VAO);
        return true;
    }

    bool unbind_VAO() const {
        glBindVertexArray(NULL_GPU_INDEX);
        return true;
    }


    ~Render_thread_data() {
    }


    void draw() {
        shader_object_.use_shader_program();

        bind_VAO();

        for (int i = 0; i < TBO_s_.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            TBO_s_.at(i).bind(); // 为什么会有多个区域呢？ 我记得好像是因为可能不同的属性会放置在不同的缓冲区

            glUniform1i(glGetUniformLocation(shader_object_.get_shader_program(),
                                             TBO_s_.at(i).get_texture_name()), i);
        }
        if (get_have_indices() == true) {
            glDrawElements(GL_TRIANGLES, get_draw_size(), GL_UNSIGNED_INT, (void *) 0);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, get_draw_size());
        }
        unbind_VAO();
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
