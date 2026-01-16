//
// Created by 潘鑫 on 2025/3/2.
//

#ifndef LABYRINTH_H
#define LABYRINTH_H

#include "model_matrix_component.h"
#include "render_component.h"
#include "input_component.h"

struct Vertex {
    Point_3 position;
    Point_3 color;
    Point_2 texCoord;
};

struct KeyEvent {
    int key_code;
    bool pressed;
};

struct Position {
    float x;
    float y;
    float z;
};

#include "set_graph.h"
#include <entt/entt.hpp>


class Labyrinth {
public:
    entt::entity entity_;

    // render_component *Labyrinth_cube;
    // Input_Component *observer;
    // Position_component *position;
    // 下面是在CPU内存上的数据，是三角的顶点和索引
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    // 如果图形有改变，那么会改变到这里，之后再
    // 等待需要绘制时，再去传输到GPU上
    // 之后再调用绘制操作绘制


    int height = 20;
    int width = 15;
    std::vector<vertex *> *graph; // 这里有图，只是计算图的话，
    std::vector<vertex *> vertices_priority_queue;
    using Labyrinth_mutex_type = std::mutex;
    Labyrinth_mutex_type change_vbo_date_mutex; // 同时只允许有一个线程更改数据


    Labyrinth(const std::string &actorName);

    /*********绘制相关操作***************/
    void init_render_object();

    void update();


    /*********绘制相关操作***************/


    bool run_init(const base_event_with_stamp &base_event);

    bool run_step(const base_event_with_stamp &base_event);

    bool set_zoom(const base_event_with_stamp &base_event);

    bool set_position_offset(const base_event_with_stamp &base_event);

    bool change_square_color(int x, int y);

    bool change_square_color_to_red(int x, int y);

    bool change_square_color_to_blue(int x, int y);

    bool deal_event(const base_event_with_stamp &base_event);

    static bool on_Event(entt::entity entity, const base_event_with_stamp &event);

    bool display_result();

    bool change_blue_color_to_red();

    ~Labyrinth();

    Labyrinth(const std::string &name, entt::entity entity);

    void add_observer();


    bool add_square();

    bool add_box(Position start_position, Position end_position);
};

#endif //LABYRINTH_H
