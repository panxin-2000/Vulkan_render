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


#define GLEW_STATIC
#include <GL/glew.h>

#include "buffer_object.h"
#include "EBO_object.h"
#include "VBO_object.h"
#include "texture_TBO.h"


class render_object {
private:
    vertex_array_VAO *VAO_new;

    Element_EBO_object EBO_object;
    Element_VBO_object VBO_object;
    Element_EBO *EBO_new;
    GLuint UBO = NULL_GPU_INDEX;
    vertex_buffer_VBO *VBO_new;
    std::vector<Texture_TBO> TBO;

    GLsizei indices_size = NULL_GPU_INDEX;
    int draw_number = 1;
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint shaderProgram; //??
    int draw_count = 0;
    bool initialized_opengl = false;

    GLsizeiptr UBO_size;
    const void *UBO_data;

public:
    render_object() {
    }

    void render_object_shader_init();

    ~render_object() {
        if (VBO_new != nullptr) {
            delete VBO_new;
        }
        if (VAO_new != nullptr) {
            delete VAO_new;
        }
        if (EBO_new != NULL_GPU_INDEX) {
            delete EBO_new;
        }
        if (UBO != NULL_GPU_INDEX) {
            glDeleteBuffers(1, &UBO);
        }
    }

    void set_VBO_parameter(GLsizeiptr VBO_size,
                           const void *VBO_data,
                           std::vector<VertexAttrib> VBO_vertex_attribs) {
        VBO_object.set_VBO_parameter(VBO_size, VBO_data, VBO_vertex_attribs);
    }

    void set_EBO_parameter(GLsizeiptr EBO_size,
                           const void *EBO_data,
                           int EBO_indices_size) {
        this->EBO_object.set_EBO_parameter(EBO_size, EBO_data, EBO_indices_size);
    }


    void set_UBO_parameter(GLsizeiptr UBO_size,
                           const void *UBO_data) {
        this->UBO_size = UBO_size;
        this->UBO_data = UBO_data;
    }


    void init_opengl_start_data() {
        if (initialized_opengl == false) {
            render_object_shader_init();
            init_and_bind_VAO();
            VBO_object.initVBO(VAO_new);
            EBO_object.initEBO(VAO_new);
            initUBO(UBO_size, UBO_data);
            initialized_opengl = true;
        }
    }

    void init_and_bind_VAO() {
        if (VAO_new == nullptr) {
            VAO_new = new vertex_array_VAO;
            VAO_new->bind();
        } else {
            VAO_new->bind();
        }
    }

    // EBO 是三角形顶点的索引，
    // EBO 其实会和绘制的元素强相关，GL_TRIANGLES GL_LINE_STRIP 应该是都不一样的，比如条带三角形


    void initUBO(GLsizeiptr size, const void *data) {
        glGenBuffers(1, &UBO);
        glBindBuffer(GL_UNIFORM_BUFFER, UBO);
        // 分配内存并传入数据（GL_STATIC_DRAW表示数据不频繁修改）
        glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, NULL_GPU_INDEX);
    }


    // 还更简洁的只更新其中一部分数据的内容


    void update_data() {
        EBO_object.update_buffer_data();
        VBO_object.update_buffer_data();
    }


    void set_draw_number(int draw_number) {
        this->draw_number = draw_number;
    }

    void draw() {
        if (VBO_object.get_draw_count() > 0) {
            glUseProgram(shaderProgram);
            for (int i = 0; i < draw_number; ++i) {
                init_and_bind_VAO();

                for (int i = 0; i < TBO.size(); ++i) {
                    glActiveTexture(GL_TEXTURE0 + i);
                    TBO.at(i).bind(); // 为什么会有多个区域呢？ 我记得好像是因为可能不同的属性会放置在不同的缓冲区
                }


                if (EBO_object.EBO_new != nullptr) {
                    glDrawElements(GL_TRIANGLES, EBO_object.get_indices_size(), GL_UNSIGNED_INT, (void *) 0);
                } else {
                    glDrawArrays(GL_LINE_STRIP, 0, VBO_object.get_draw_count()); //count还是需要去获取的
                }
                glBindVertexArray(NULL_GPU_INDEX);
            }
        }
    }
};


#endif //CUBE_H
