//
// Created by 潘鑫 on 2026/3/8.
//


#include "Rect_2D_component.h"
#include "shader_component.h"
#include "VKR_proxy_component.h"

void sync_render_data_to_render_thread() {
    // 应该不止更新 position，还有很多的都需要更新
    global_uniform_buffer_update_function();
    uniform_buffer_update_function();
    descriptor_set_update_function();
    add_new_peoxy_to_render_function();
    update_2D_UI_object_function();
}
