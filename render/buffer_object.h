//
// Created by 潘鑫 on 2025/11/29.
//

#ifndef LEARN_OPENGL_BUFFER_OBJECT_H
#define LEARN_OPENGL_BUFFER_OBJECT_H

#define GLEW_STATIC
#include <GL/glew.h>

#include "VAO_object.h"

template<GLuint type_value = GL_ELEMENT_ARRAY_BUFFER>
class GL_buffer_object {
public:
    void bind() const {
        glBindBuffer(type_value, buffer_object);
    }

    void un_bind() const {
        glBindBuffer(type_value, NULL_GPU_INDEX);
    }

    GLuint buffer_object = NULL_GPU_INDEX;
    const vertex_array_VAO *VAO_new;

    GL_buffer_object(const vertex_array_VAO *temp_VAO) {
        VAO_new = temp_VAO;
        glGenBuffers(1, &buffer_object);
    }

    ~GL_buffer_object() {
        if (buffer_object != NULL_GPU_INDEX) {
            glDeleteBuffers(1, &buffer_object);
        }
    }

    void update_buffer_data(GLsizeiptr size, const void *data) {
        VAO_new->bind();
        this->bind();
        glBufferData(type_value, size, data, GL_STATIC_DRAW);
        VAO_new->un_bind();
        this->un_bind();
    }
};

#endif //LEARN_OPENGL_BUFFER_OBJECT_H
