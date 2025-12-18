//
// Created by 潘鑫 on 2025/3/2.
//

#ifndef LABYRINTH_H
#define LABYRINTH_H

#include "base_event.h"
#include "../render/render_object.h"

struct Vertex {
    float position[3];
    float color[3];
    float texCoord[2];
};

struct position {
    float x;
    float y;
    float z;
};

#include "set_graph.h"

class Labyrinth {
public:
    render_object *Labyrinth_cube;

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


    Labyrinth();

    /*********绘制相关操作***************/
    void init_render_object();

    void update();


    /*********绘制相关操作***************/


    void run_init(const base_event_with_stamp &base_event);

    void run_step(const base_event_with_stamp &base_event);

    bool change_square_color(int x, int y);

    bool change_square_color_to_red(int x, int y);

    bool change_square_color_to_blue(int x, int y);

    void deal_event(const base_event_with_stamp &base_event);

    bool display_result();

    bool change_blue_color_to_red();

    ~Labyrinth();


    bool add_square();

    bool add_box(position start_position, position end_position);
};

#endif //LABYRINTH_H
