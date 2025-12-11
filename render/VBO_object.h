//
// Created by 潘鑫 on 2025/11/29.
//

#ifndef LEARN_OPENGL_VBO_OBJECT_H
#define LEARN_OPENGL_VBO_OBJECT_H
#include "buffer_object.h"
#include <vector>

class vertex_buffer_VBO : public GL_buffer_object<GL_ARRAY_BUFFER> {
public:
    vertex_buffer_VBO() = delete;


    vertex_buffer_VBO(const vertex_array_VAO *temp_VAO)
        : GL_buffer_object(temp_VAO) {
    }

    int update_vbo_data_attribute(GLsizeiptr size, std::vector<VertexAttrib> &vertex_attribs) {
        VAO_new->bind();
        this->bind();
        auto draw_count = 0;
        // 下面这个可以配置一个结构体或者其他内容来实现快速输入
        for (int i = 0; i < vertex_attribs.size(); ++i) {
            glVertexAttribPointer(i, vertex_attribs[i].size, vertex_attribs[i].type,
                                  vertex_attribs[i].normalized, vertex_attribs[i].stride, vertex_attribs[i].pointer);
            glEnableVertexAttribArray(i);
            draw_count = size / vertex_attribs[i].stride;
        }
        VAO_new->un_bind();
        this->un_bind();

        return draw_count;
    }

    void update_buffer_data(GLsizeiptr size, const void *data) {
        VAO_new->bind(); // 只是更新这里时，这里应该是不需要绑定的，但是绑定上也是没有错的的
        this->bind();
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
        VAO_new->un_bind();
        this->un_bind();
    }
};


struct Element_VBO_detail {
    GLsizeiptr VBO_size;
    const void *VBO_data;
    std::vector<VertexAttrib> vertex_attribs;
};

struct Element_VBO_object {
    vertex_buffer_VBO *VBO_new;
    Element_VBO_detail detail[2];
    bool currently_active;
    bool need_update = false;

#define  update_size            detail[!currently_active].VBO_size
#define  update_data            detail[!currently_active].VBO_data
#define  update_vertex_attribs  detail[!currently_active].vertex_attribs
#define  render_size            detail[currently_active].VBO_size
#define  render_data            detail[currently_active].VBO_data
#define  render_vertex_attribs  detail[currently_active].vertex_attribs


    void set_VBO_parameter(GLsizeiptr VBO_size,
                           const void *VBO_data,
                           std::vector<VertexAttrib> VBO_vertex_attribs) {
        update_size = VBO_size;
        update_data = VBO_data;
        update_vertex_attribs = VBO_vertex_attribs;
        need_update = true;
    }

    void update_buffer_data() {
        // if (need_update) {
        currently_active = !currently_active;
        VBO_new->update_buffer_data(render_size,render_data);
        need_update = false;
        // }
    }

    int get_draw_count() {
        if (render_vertex_attribs.size() > 0) {
            auto temp = detail[currently_active].vertex_attribs[0].stride;
            return render_size / temp;
        }
        return 0;
    }

    void initVBO(vertex_array_VAO *VAO_new) {
        if (VBO_new != nullptr) {
        } else {
            VBO_new = new vertex_buffer_VBO(VAO_new);
        }
        update_buffer_data();
        VBO_new->update_vbo_data_attribute(render_size, render_vertex_attribs);
    }

#undef update_size
#undef update_data
#undef update_vertex_attribs
#undef render_size
#undef render_data
#undef render_vertex_attribs
};


#endif //LEARN_OPENGL_VBO_OBJECT_H
