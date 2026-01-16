//
// Created by 潘鑫 on 2025/11/27.
//

#include "render_object_manage.h"

/**
 * 添加 render_object 到管理，并进行初始化，现在是加入时就初始化，
 * 之后，复杂了之后，可以是需要渲染时再初始化并渲染
 * @param render_object
 * @return
 */
bool add_object_to_render_manager(logic_render_data *render_object) {
    render_object_manage::get_instance().add_render_object(render_object);
    return true;
}

/**
 * 通知渲染管理器，检查并更新 渲染对象
 * @return
 */
bool notify_render_manager_update_objects() {
    render_object_manage::get_instance().change_update_status(true);
    return true;
}

void start_render_manage_thread(GLFWwindow *window) {
    render_object_manage::get_instance().render_thread(window);
}

void end_render_manage_thread() {
    render_object_manage::get_instance().render_thread_stop();
}
