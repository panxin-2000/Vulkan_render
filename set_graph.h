//
// Created by 潘鑫 on 2025/2/18.
//

#ifndef SET_GRAPH_H
#define SET_GRAPH_H

struct ee {
    int a;
    int b;
    bool operator==(const ee& lhs)const
    {
        return lhs.a == a && lhs.b == b;
    }
};


//还需要几个函数
//添加上下左右
//删除上下左右
// 清除整个图
enum direction {
    up, down, left, right
};

void add_edge(std::list<ee> *list, int x, int y, enum direction d);

/**
 * 删除其中一条边搞定了，但是删除需要删除相互的两条边
 * @param graph
 * @param x
 * @param y
 * @param d
 */
void delete_edge(std::vector<std::list<ee> *> graph, int x, int y, enum direction d);

std::vector<std::list<ee> *> *init_graph(int square_length);
#endif //SET_GRAPH_H
