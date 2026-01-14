//
// Created by 潘鑫 on 2025/3/2.
//
#include "labyrinth.h"
#include "base_event.h"
#include "ECS.h"
#include "entity_name_component.h"
#include "observer_manage.h"
#include "render_object_manage.h"


void on_key_press(const KeyEvent &event) {
    if (event.key_code == 27) /* 处理退出逻辑 */;
}

Labyrinth::Labyrinth(const std::string &name) {
    init_render_object();

    /***************创建*******************/
    entity = get_entt_instance().create();
    get_entt_instance().emplace<render_component>(entity);
    get_entt_instance().emplace<Input_Component>(entity);
    get_entt_instance().emplace<Position_component>(entity);
    get_entt_instance().emplace<Drag_event>(entity);
    get_entt_instance().emplace<Name_component>(entity, name);
    auto view = get_entt_instance().view<Position_component>();
    std::cout << "View size: " << view.size() << std::endl;

    // observer->set_deal_function(std::bind(&Labyrinth::deal_event, this, std::placeholders::_1));
    // observe_manage_instance::instance().addObserver(*observer);
    // base_observer<base_event> observer{EventType::mouse_release_left, "mouse_release_left"};
    // observer.set_deal_function(std::bind(&Labyrinth::deal_event, this, std::placeholders::_1));


    /***************设置参数**********************/
    std::vector<VertexAttrib> vertex_attribs;
    vertex_attribs.emplace_back(3,GL_FLOAT,GL_FALSE, sizeof(Vertex), (void *) 0);
    vertex_attribs.emplace_back(3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) (3 * sizeof(float)));
    vertex_attribs.emplace_back(2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) (6 * sizeof(float)));

    auto &position = get_entt_instance().get<Position_component>(entity);

    position.update_position();


    // 参数这里最重要的是下面的两行
    auto &Labyrinth_cube = get_entt_instance().get<render_component>(entity);

    Labyrinth_cube.add_texture_path("resoureces/picture.png", "ourTexture1");
    Labyrinth_cube.set_VBO_parameter(vertices.size() * sizeof(Vertex), vertices.data(), vertex_attribs);
    Labyrinth_cube.set_EBO_parameter(indices.size() * sizeof(GLuint), indices.data(), indices.size());
    Labyrinth_cube.set_vertex_shader("render/shader/labyrinth.vert");
    Labyrinth_cube.set_fragment_shader("render/shader/labyrinth.frag");
    /***************添加到渲染管理器**********************/
    add_object_to_render_manager(&Labyrinth_cube);


    graph = init_graph(height, width);
    add_observer();
}

void Labyrinth::add_observer() {
    // auto &observer = get_entt_instance().get<Input_Component>(entity);

    // observer.Subscribe_Event(EventType::key_combination, "'a'",
    // [this](auto &&PH1) { return run_step(std::forward<decltype(PH1)>(PH1)); });
    // observer.Subscribe_Event(EventType::key_combination, "'q'",
    // [this](auto &&PH1) { return run_init(std::forward<decltype(PH1)>(PH1)); });
    // observer.Subscribe_Event(EventType::mouse_release_left, "mouse_release_left",
    // [this](auto &&PH1) { return deal_event(std::forward<decltype(PH1)>(PH1)); });
}


Labyrinth::~Labyrinth() {
    delete_graph(*graph);
}

void Labyrinth::update() {
    // std::unique_lock<std::mutex> lock(change_vbo_date_mutex);
    // 加锁保证在传输时不会出现更改，防止撕裂

    /***************更新参数**********************/
    std::vector<VertexAttrib> vertex_attribs;
    vertex_attribs.emplace_back(3,GL_FLOAT,GL_FALSE, sizeof(Vertex), (void *) 0);
    vertex_attribs.emplace_back(3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) (3 * sizeof(float)));

    auto &Labyrinth_cube = get_entt_instance().get<render_component>(entity);
    Labyrinth_cube.set_VBO_parameter(vertices.size() * sizeof(Vertex), vertices.data(), vertex_attribs);
    /***************通知管理器更新渲染对象**********************/
    notify_render_manager_update_objects();
}


bool Labyrinth::run_init(const base_event_with_stamp &base_event) {
    std::lock_guard<Labyrinth_mutex_type> lock(change_vbo_date_mutex);
    dijkstra_init(*graph, {0, 0}, {width - 1, height - 1},
                  vertices_priority_queue);
    change_blue_color_to_red();
    update();
}

bool Labyrinth::run_step(const base_event_with_stamp &base_event) {
    std::lock_guard<Labyrinth_mutex_type> lock(change_vbo_date_mutex);
    auto u = dijkstra_step(*graph, vertices_priority_queue);
    if (u != nullptr) {
        change_square_color_to_blue(u->self_position.x_position, u->self_position.y_position);
    }
    update();
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


bool Labyrinth::add_box(Position start_position, Position end_position) {
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


// x: -0.936982 y: 0.948659
// x: -0.936982 y: 0.948659
//  offset x: 0 y: 0
// x: -0.936982 y: 0.94944
// x: -0.552598 y: 0.514779
//  offset x: 0.38499 y: -0.434661
// x: -0.552598 y: 0.514779
// x: -0.552598 y: 0.514779
//  offset x: 0.38499 y: -0.434661

bool Labyrinth::deal_event(const base_event_with_stamp &base_event) {
    std::lock_guard<Labyrinth_mutex_type> lock(change_vbo_date_mutex);
    // 有一点内容需要明确，offset 其实应该指的是迷宫方块左下角的坐标
    auto &position = get_entt_instance().get<Position_component>(entity);
    auto offset = position.get_offset();
    auto zoom = position.get_zoom();

    auto x = (base_event.data.select_box.click_pos.x - offset.x) / zoom.x;
    auto y = (base_event.data.select_box.click_pos.y - offset.y) / zoom.y;
    int x_int = (x + 1) / 2 * width;
    int y_int = (-y + 1) / 2 * height;
    auto e_x = (base_event.data.select_box.release_pos.x - offset.x) / zoom.x;
    auto e_y = (base_event.data.select_box.release_pos.y - offset.y) / zoom.y;
    int e_x_int = (e_x + 1) / 2 * width;
    int e_y_int = (-e_y + 1) / 2 * height;
    if (x_int == e_x_int && y_int == e_y_int) {
        change_square_color(x_int, y_int);
        update();
    }
}

bool Labyrinth::change_square_color(int x, int y) {
    if (x > width || y > height || x < 0 || y < 0) {
        return false;
    }
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
