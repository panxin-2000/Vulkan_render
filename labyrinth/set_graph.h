//
// Created by 潘鑫 on 2025/2/18.
//

#ifndef SET_GRAPH_H
#define SET_GRAPH_H
#include <list>

struct vertex_position {
    int x_position;
    int y_position;
    double weight;

    bool operator==(const vertex_position &lhs) const {
        return lhs.x_position == x_position && lhs.y_position == y_position;
    }
};


//还需要几个函数
//添加上下左右
//删除上下左右
// 清除整个图
enum direction {
    up_x_decrease, down_x_add, left_y_decrease, right_y_add
};

struct vertex {
    vertex_position self_position; //设置图时设置
    vertex *parent = nullptr; //更新distance时同步更新
    double distance; //除起始点外，全部设置为无穷大
    std::list<vertex_position> edge_list; //这里面的内容可以不是 vertex_position 吗？
    //不是 也行，创建和删除时会比较麻烦，需要改动
    bool operator()(const vertex *l, const vertex *r) const { return l->distance > r->distance; }
};

void add_edge(std::list<vertex_position> &edge_list, int x, int y, enum direction d);

vertex &get_vertex(const std::vector<vertex *> &graph, int x, int y);

vertex &get_vertex(const std::vector<vertex *> &graph, const vertex_position vertex);

/**
 * 删除其中一条边搞定了，但是删除需要删除相互的两条边
 * @param graph
 * @param x
 * @param y
 * @param d
 */
void delete_edge(std::vector<vertex *> &graph, int x, int y, enum direction d);

void add_edge(std::vector<vertex *> &graph, int x, int y, enum direction d);

std::vector<vertex *> *init_graph(int square_y, int square_x);

void dijkstra(const std::vector<vertex *> &graph, vertex_position start, vertex_position end);

void dijkstra_init(const std::vector<vertex *> &graph, vertex_position start, vertex_position end,
                   std::vector<vertex *> &vertices_priority_queue);

vertex *dijkstra_step(const std::vector<vertex *> &graph, std::vector<vertex *> &vertices_priority_queue);

bool delete_graph(const std::vector<vertex *> &graph);
#endif //SET_GRAPH_H
