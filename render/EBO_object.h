//
// Created by 潘鑫 on 2025/11/29.
//

#ifndef LEARN_OPENGL_EBO_OBJECT_H
#define LEARN_OPENGL_EBO_OBJECT_H

#include "VAO_object.h"
#include "buffer_object.h"


using Element_EBO = GL_buffer_object<GL_ELEMENT_ARRAY_BUFFER>;

struct Element_EBO_detail {
    GLsizeiptr EBO_size;
    const void *EBO_data;
    int EBO_indices_size;
};

struct Element_EBO_object {
    Element_EBO *EBO_new;
    Element_EBO_detail detail[2];
    bool currently_active;
    bool need_update = false;
#define  update_size          detail[!currently_active].EBO_size
#define  update_data          detail[!currently_active].EBO_data
#define  update_indices_size  detail[!currently_active].EBO_indices_size
#define  render_size          detail[currently_active].EBO_size
#define  render_data          detail[currently_active].EBO_data
#define  render_indices_size  detail[currently_active].EBO_indices_size

    void set_EBO_parameter(GLsizeiptr EBO_size,
                           const void *EBO_data,
                           int EBO_indices_size) {
        update_size = EBO_size;
        update_data = EBO_data;
        update_indices_size = EBO_indices_size;
        need_update = true;
    }

    int get_indices_size() {
        return render_indices_size;
    }

    void update_buffer_data() {
        if (need_update) {
            currently_active = !currently_active;
            // 已经交互，其他线程更改不了，不会出现撕裂，
            // 另一个问题逻辑线程更改到一半是渲染线程开始调用渲染
            // 这里通过另一个办法去解决，need_update表示更新完成
            EBO_new->update_buffer_data(render_size,render_data);
            need_update = false;
        }
    }

    void initEBO(vertex_array_VAO *VAO_new) {
        if (need_update) {
            if (EBO_new != nullptr) {
            } else {
                EBO_new = new GL_buffer_object(VAO_new);
            }
            update_buffer_data();
        }
    }

#undef update_size
#undef update_data
#undef update_indices_size
#undef render_size
#undef render_data
#undef render_indices_size
};

#endif //LEARN_OPENGL_EBO_OBJECT_H
