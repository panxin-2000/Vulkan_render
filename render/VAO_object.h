//
// Created by 潘鑫 on 2025/11/29.
//

#ifndef LEARN_OPENGL_VAO_OBJECT_H
#define LEARN_OPENGL_VAO_OBJECT_H

#define GLEW_STATIC
#include <GL/glew.h>
#define NULL_GPU_INDEX 0


struct VertexAttrib {
    GLint size;
    GLenum type;
    GLboolean normalized;
    GLsizei stride;
    const void *pointer;

    /**
     *
     * @param size 表示有几个数据
     * @param type 类型，表示其中单个数据的类型
     * @param normalized 是否需要归一化
     * @param stride 间隔，重新下一个数据需要间隔多远
     * @param pointer 访问时是否需要便宜
     */
    VertexAttrib(GLint size,
                 GLenum type,
                 GLboolean normalized,
                 GLsizei stride,
                 const void *pointer
    ) : size(size), type(type), normalized(normalized), stride(stride), pointer(pointer) {
    }
};

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
