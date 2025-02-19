//
// Created by 潘鑫 on 2025/2/18.
//

#ifndef SET_GRAPH_H
#define SET_GRAPH_H

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
    up, down, left, right
};

struct vertex {
    vertex_position self_position;           //设置图时设置
    vertex *parent = nullptr;                //更新distance时同步更新
    double distance;                         //除起始点外，全部设置为无穷大
    double weight;                           //初始化时设置
    std::list<vertex_position> edge_list;    //这里面的内容可以不是 vertex_position 吗？
                                             //不是 也行，创建和删除时会比较麻烦，需要改动
    bool operator()(const vertex *l, const vertex *r) const { return l->distance > r->distance; }
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

void dijkstra(const std::vector<vertex *> &graph, vertex_position start, vertex_position end) ;

bool delete_graph(const std::vector<vertex *> &graph);
#endif //SET_GRAPH_H
