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
bool add_object_to_render(logic_render_data *render_object) {
    render_object_manage::get_instance().add_render_object_need_init(render_object);
    return true;
}

bool update_object_to_render(logic_render_data *render_object) {
    render_object_manage::get_instance().add_render_object_need_update(render_object);
    return true;
}

bool clean_object_to_render(logic_render_data *render_object) {
    render_object_manage::get_instance().add_render_object_need_clean(render_object);
    return true;
}


void start_render_manage_thread(GLFWwindow *window) {
    render_object_manage::get_instance().render_thread(window);
}

void end_render_manage_thread() {
    render_object_manage::get_instance().render_thread_stop();
}


void render_object_manage::init_logic_need_resources() {
    for (auto user_render_component: need_init) {
        Shader_object::create_vertex_shader(user_render_component->vertexPath_, &vertex_shader_map_);
        Shader_object::create_fragment_shader(user_render_component->fragmentPath_, &fragment_shader_map_);
        Shader_object::create_geometry_shader(user_render_component->geometryPath_, &geometry_shader_map_);
        create_vertex_buffer(user_render_component->vertices_, &vertices_map_);
        create_element_buffer(user_render_component->indices_, &indices_map_);

        // 内容都创建完成了。之后应该怎么做呢？ 绑定。
    }
}

void render_object_manage::init_VAO_bind_buffer() {
    for (auto user_render_component: need_init) {
        union_render_data data;
        data.logic_data = user_render_component;
        data.render_data = new Render_thread_data;
        data.render_data->create_VAO();
        data.render_data->bind_VAO();
        bind_vertex_buffer(user_render_component->vertices_, &vertices_map_);
        int stride = 0;
        for (const auto &vertex_attrib: user_render_component->vertex_attribs) {
            stride += vertex_attrib.size * get_glenum_length(vertex_attrib.type);
        }
        int pointer = 0;
        for (int i = 0; i < user_render_component->vertex_attribs.size(); ++i) {
            glVertexAttribPointer(i, user_render_component->vertex_attribs[i].size,
                                  user_render_component->vertex_attribs[i].type,
                                  user_render_component->vertex_attribs[i].normalized,
                                  stride,
                                  (const void *) pointer);
            glEnableVertexAttribArray(i);
            pointer += user_render_component->vertex_attribs[i].size *
                    get_glenum_length(user_render_component->vertex_attribs[i].type);
        }

        bind_element_buffer(user_render_component->indices_, &indices_map_);
        data.render_data->unbind_VAO();
        if (user_render_component->indices_ != nullptr)
            data.render_data->set_draw_size(user_render_component->indices_->size());
        else
            data.render_data->set_draw_size(user_render_component->vertices_->size());

        for (const auto &texture_logic: user_render_component->textures) {
            Texture_TBO temp;
            temp.set_path(texture_logic.path_, texture_logic.texture_name_);
            // temp.set_texture(); // 需要查找后设置。
        }

        data.render_data->shader_object_.shader_init_and_attach(user_render_component,
                                                                &vertex_shader_map_,
                                                                &fragment_shader_map_,
                                                                &geometry_shader_map_);
        data.render_data->shader_object_.update_uniforms(&user_render_component->uniforms_map);
        render_objects.push_back(data);
    }
}

render_object_manage &render_object_manage::get_instance() {
    static render_object_manage *instance = nullptr;
    static std::once_flag flag;
    // 线程安全：由系统保证 inside 的 lambda 只执行一次
    std::call_once(flag, []() {
        instance = new render_object_manage();
    });
    return *instance;
}

void render_object_manage::add_render_object_need_init(logic_render_data *render_object) {
    std::unique_lock<std::mutex> lock(mtx);
    need_init.push_back(render_object);
}

void render_object_manage::add_render_object_need_update(logic_render_data *render_object) {
    std::unique_lock<std::mutex> lock(mtx);
    need_update.push_back(render_object);
}

void render_object_manage::add_render_object_need_clean(logic_render_data *render_object) {
    std::unique_lock<std::mutex> lock(mtx);
    need_clean.push_back(render_object);
}
