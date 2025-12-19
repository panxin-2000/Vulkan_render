//
// Created by 潘鑫 on 2025/3/2.
//
#include "labyrinth.h"

#include "base_event.h"
#include "render_object_manage.h"

Labyrinth::Labyrinth() {
    init_render_object();

    /***************创建*******************/
    Labyrinth_cube = new render_object();

    /***************设置参数**********************/
    std::vector<VertexAttrib> vertex_attribs;
    vertex_attribs.emplace_back(3,GL_FLOAT,GL_FALSE, sizeof(Vertex), (void *) 0);
    vertex_attribs.emplace_back(3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) (3 * sizeof(float)));

    Shader_object::data_value_or_ptr data;
    data.float_val = std::pow(1.5, zoom);
    Labyrinth_cube->add_uniform("value", Shader_object::gl_float, data);

    // 参数这里最重要的是下面的两行
    Labyrinth_cube->set_VBO_parameter(vertices.size() * sizeof(Vertex), vertices.data(), vertex_attribs);
    Labyrinth_cube->set_EBO_parameter(indices.size() * sizeof(GLuint), indices.data(), indices.size());
    Labyrinth_cube->set_vertex_shader("render/shader/labyrinth.vert");
    Labyrinth_cube->set_fragment_shader("render/shader/labyrinth.frag");
    /***************添加到渲染管理器**********************/
    add_object_to_render_manager(Labyrinth_cube);


    graph = init_graph(height, width);
}

Labyrinth::~Labyrinth() {
    delete Labyrinth_cube;
    delete_graph(*graph);
}

void Labyrinth::update() {
    // std::unique_lock<std::mutex> lock(change_vbo_date_mutex);
    // 加锁保证在传输时不会出现更改，防止撕裂

    /***************更新参数**********************/
    std::vector<VertexAttrib> vertex_attribs;
    vertex_attribs.emplace_back(3,GL_FLOAT,GL_FALSE, sizeof(Vertex), (void *) 0);
    vertex_attribs.emplace_back(3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) (3 * sizeof(float)));

    Labyrinth_cube->set_VBO_parameter(vertices.size() * sizeof(Vertex), vertices.data(), vertex_attribs);
    /***************通知管理器更新渲染对象**********************/
    notify_render_manager_update_objects();
}


void Labyrinth::run_init(const base_event_with_stamp &base_event) {
    std::lock_guard<Labyrinth_mutex_type> lock(change_vbo_date_mutex);
    dijkstra_init(*graph, {0, 0}, {width - 1, height - 1},
                  vertices_priority_queue);
    change_blue_color_to_red();
    update();
}

void Labyrinth::run_step(const base_event_with_stamp &base_event) {
    std::lock_guard<Labyrinth_mutex_type> lock(change_vbo_date_mutex);
    auto u = dijkstra_step(*graph, vertices_priority_queue);
    if (u != nullptr) {
        change_square_color_to_blue(u->self_position.x_position, u->self_position.y_position);
    }
    update();
}

void Labyrinth::set_zoom(const base_event_with_stamp &base_event) {
    zoom = zoom + base_event.data.scroll.y * 0.01;
    Shader_object::data_value_or_ptr data;
    data.float_val = std::pow(1.5, zoom);
    Labyrinth_cube->add_uniform("value", Shader_object::gl_float, data);
}


void Labyrinth::init_render_object() {
    float x_size = 2.0f / width;
    float y_size = 2.0f / height;
    float p = 0.8;
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            float x = -1 + x_size * i + x_size / 2;
            float y = 1 - y_size * j - y_size / 2;
            add_box({x - x_size * 0.4f, y - y_size * 0.4f, 0.0}, {x + x_size * 0.4f, y + y_size * 0.4f, 0.0});
        }
    }
}


bool Labyrinth::add_box(position start_position, position end_position) {
    Vertex vertex; //一定会有四个点，分别是x最大和x最小，y最大和y最小
    // x小y大 2     x大y大 3
    // x小y小 0     x大y小 1
    // 默认情况下，逆时针的顶点连接顺序被定义为三角形的正面
    indices.push_back(vertices.size() + 0);
    indices.push_back(vertices.size() + 1);
    indices.push_back(vertices.size() + 2);
    indices.push_back(vertices.size() + 1);
    indices.push_back(vertices.size() + 3);
    indices.push_back(vertices.size() + 2);
    vertex.position.x = start_position.x < end_position.x ? start_position.x : end_position.x;
    vertex.position.y = start_position.y < end_position.y ? start_position.y : end_position.y;
    vertex.position.z = 0;
    vertex.color.x = 0.5f;
    vertex.color.y = 0;
    vertex.color.z = 0;
    vertex.texCoord.x = 0;
    vertex.texCoord.y = 0;
    vertices.push_back(vertex);
    vertex.position.x = start_position.x > end_position.x ? start_position.x : end_position.x;
    vertex.position.y = start_position.y < end_position.y ? start_position.y : end_position.y;
    vertex.texCoord.x = 1;
    vertex.texCoord.y = 0;
    vertices.push_back(vertex);
    vertex.position.x = start_position.x < end_position.x ? start_position.x : end_position.x;
    vertex.position.y = start_position.y > end_position.y ? start_position.y : end_position.y;
    vertex.texCoord.x = 0;
    vertex.texCoord.y = 1;
    vertices.push_back(vertex);
    vertex.position.x = start_position.x > end_position.x ? start_position.x : end_position.x;
    vertex.position.y = start_position.y > end_position.y ? start_position.y : end_position.y;
    vertex.texCoord.x = 1;
    vertex.texCoord.y = 1;
    vertices.push_back(vertex);
}


void Labyrinth::deal_event(const base_event_with_stamp &base_event) {
    std::lock_guard<Labyrinth_mutex_type> lock(change_vbo_date_mutex);
    // ypos = 300 - ypos;
    auto x = base_event.data.pos.x;
    auto y = base_event.data.pos.y;
    change_square_color((int) (x / 800 * width), (int) (y / 600 * height));
    // have_change_data = true;
    update();
}

bool Labyrinth::change_square_color(int x, int y) {
    int a = x + y * width;
    // 不再对颜色进行重置
    if (vertices[a * 4].color.y == 1.0f) {
        add_edge(*graph, x, y, up_x_decrease);
        add_edge(*graph, x, y, right_y_add);
        add_edge(*graph, x, y, left_y_decrease);
        add_edge(*graph, x, y, down_x_add);
        for (int i = a * 4; i < (a + 1) * 4; ++i) {
            vertices[i].color = {0.5f, 0.0f, 0.0f};
        }
    } else {
        for (int i = a * 4; i < (a + 1) * 4; ++i) {
            vertices[i].color = {0.0f, 1.0f, 0.0f};
        }
        delete_edge(*graph, x, y, up_x_decrease);
        delete_edge(*graph, x, y, right_y_add);
        delete_edge(*graph, x, y, left_y_decrease);
        delete_edge(*graph, x, y, down_x_add);
    }
}

bool Labyrinth::display_result() {
    vertex s = get_vertex(*graph, {width - 1, width - 1});
    vertex *ss = &s;
    while (1) {
        if (ss->parent != nullptr) {
            change_square_color_to_blue(ss->self_position.x_position, ss->self_position.y_position);
            ss = ss->parent;
        } else {
            break;
        }
    }
    change_square_color_to_red(0, 0);
    change_square_color_to_blue(width - 1, width - 1);
}

bool Labyrinth::change_square_color_to_red(int x, int y) {
    int a = x + y * width;
    for (int i = a * 4; i < (a + 1) * 4; ++i) {
        vertices[i].color = {1.0f, 0.0f, 0.0f};
    }
}

bool Labyrinth::change_square_color_to_blue(int x, int y) {
    int a = x + y * width;
    for (int i = a * 4; i < (a + 1) * 4; ++i) {
        vertices[i].color = {0.0f, 0.0f, 1.0f};
    }
}

bool Labyrinth::change_blue_color_to_red() {
    for (auto &vertex: vertices) {
        if (vertex.color.z == 1.0f) {
            vertex.color = {0.5f, 0.0f, 0.0f};
        }
    } // 不再对颜色进行重置
}
