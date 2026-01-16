//
// Created by 潘鑫 on 2025/11/29.
//

#ifndef LEARN_OPENGL_VAO_OBJECT_H
#define LEARN_OPENGL_VAO_OBJECT_H

#define GLEW_STATIC
#include <GL/glew.h>
#include "shader_common.h"

class vertex_array_VAO {
    GLuint buffer_object = NULL_GPU_INDEX;

public:
    void bind() const {
        glBindVertexArray(buffer_object);
    }

    void un_bind() const {
        glBindVertexArray(NULL_GPU_INDEX);
    }

    vertex_array_VAO() {
        glGenVertexArrays(1, &buffer_object);
    }

    ~vertex_array_VAO() {
        glDeleteVertexArrays(1, &buffer_object);
    }
};


#endif //LEARN_OPENGL_VAO_OBJECT_H
