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
#include "shader.h"

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
    Shader_object shader_object;
    int draw_count = 0;
    bool initialized_opengl = false;

    GLsizeiptr UBO_size;
    const void *UBO_data;

public:
    render_object() {
    }

    bool add_texture_path(char const *path, char const *texture_name) {
        Texture_TBO texture;
        texture.set_path(path, texture_name);
        TBO.push_back(texture);
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

    void set_vertex_shader(const char *path) {
        shader_object.set_vertex_shader(path);
    }

    void set_fragment_shader(const char *path) {
        shader_object.set_fragment_shader(path);
    }

    void set_geometry_shader(const char *path) {
        shader_object.set_geometry_shader(path);
    }

    void add_uniform(const std::string &name, Shader_object::Uniforms_type uniforms_type,
                     Shader_object::data_value_or_ptr &data,
                     uint8_t number = 1) {
        shader_object.add_uniform(name, uniforms_type, data, number);
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
            shader_object.Shader_init();
            init_and_bind_VAO();
            VBO_object.initVBO(VAO_new);
            EBO_object.initEBO(VAO_new);
            initUBO(UBO_size, UBO_data);
            for (int i = 0; i < TBO.size(); ++i) {
                TBO.at(i).loadTexture();
            }
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
            shader_object.use_shader_program();
            // 之后就是
            // GLint viewLoc = glGetUniformLocation(skyboxShader.Program, "view");
            // GLint cubemap = glGetUniformLocation(skyboxShader.Program, "cubemap");
            // glUniform1i(cubemap, 0);
            // GLint viewLoc = glGetUniformLocation(skyboxShader.Program, "view");
            // glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const GLfloat *) &view);

            for (int i = 0; i < draw_number; ++i) {
                init_and_bind_VAO();
                shader_object.update_uniforms();

                for (int i = 0; i < TBO.size(); ++i) {
                    glActiveTexture(GL_TEXTURE0 + i);
                    TBO.at(i).bind(); // 为什么会有多个区域呢？ 我记得好像是因为可能不同的属性会放置在不同的缓冲区
                    glUniform1i(glGetUniformLocation(shader_object.get_shader_program(),
                                                     TBO.at(i).get_texture_name()),
                                i);
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
