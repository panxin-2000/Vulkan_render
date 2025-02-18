//
// Created by 潘鑫 on 2025/2/18.
//

#ifndef SET_GRAPH_H
#define SET_GRAPH_H

struct vertex_position {
    int x_position;
    int y_position;

    bool operator==(const vertex_position &lhs) const {
        return lhs.x_position == x_position && lhs.y_position == y_position;
    }
};


//还需要几个函数
//添加上下左右
//删除上下左右
// 清除整个图
enum direction {
    up, down, left, right
};

struct vertex {
    vertex_position self_position;
    vertex_position *parent = nullptr;
    double distance;
    double weight;
    std::list<vertex_position> edge_list;
};

void add_edge(std::list<vertex_position> &edge_list, int x, int y, enum direction d);

/**
 * 删除其中一条边搞定了，但是删除需要删除相互的两条边
 * @param graph
 * @param x
 * @param y
 * @param d
 */
void delete_edge(std::vector<vertex *> &graph, int x, int y, enum direction d);

std::vector<vertex *> *init_graph(int square_length);

bool delete_graph(const std::vector<vertex *> &graph);
#endif //SET_GRAPH_H
