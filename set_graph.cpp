//
// Created by 潘鑫 on 2025/2/18.
//
#include <vector>
#include <list>
#include "set_graph.h"

/**
 * 这里是逻辑，逻辑不变，但是数据的方向是可以改的
 * @param list
 * @param x
 * @param y
 * @param d
 */
void add_edge(std::list<ee> *list, int x, int y, enum direction d) {
    if (d == up) {
        struct ee tem = {x - 1, y}; // 上
        list->push_back(tem);
    } else if (d == down) {
        struct ee tem = {x + 1, y}; //下
        list->push_back(tem);
    } else if (d == left) {
        struct ee tem = {x, y - 1}; // 左
        list->push_back(tem);
    } else if (d == right) {
        struct ee tem = {x, y + 1}; //右
        list->push_back(tem);
    }
}

void delete_edge(std::vector<std::list<ee> *> graph, int x, int y, enum direction d) {
    auto list = graph.at(x * 10 + y);
    if (d == up) {
        for (struct ee tem: *list) {
            if (tem.a == x - 1 && tem.b == y) {
                list->remove(tem);
                delete_edge(graph, x - 1, y, down);
                break;
            }
        }
    } else if (d == down) {
        for (struct ee tem: *list) {
            if (tem.a == x + 1 && tem.b == y) {
                list->remove(tem);
                delete_edge(graph, x + 1, y, up);
                break;
            }
        }
    } else if (d == left) {
        for (struct ee tem: *list) {
            if (tem.a == x && tem.b == y - 1) {
                list->remove(tem);
                delete_edge(graph, x, y - 1, right);
                break;
            }
        }
    } else if (d == right) {
        for (struct ee tem: *list) {
            if (tem.a == x && tem.b == y + 1) {
                list->remove(tem);
                delete_edge(graph, x, y + 1, left);
                break;
            }
        }
    }
}

std::vector<std::list<ee> *> *init_graph(int square_length) {
    auto t3 = new std::vector<std::list<ee> *>;
    for (int x = 0; x < square_length; ++x) {
        for (int y = 0; y < square_length; ++y) {
            auto list = new std::list<ee>; //这里内存应该是释放了的。
            // 首先是四个角，只有两个可以到达的结点
            // 然后是四条边不包含四个角，每个有三个
            // 最后是中间的内容，都可以到达
            if (x == 0 && y == 0) {
                // 左上角
                add_edge(list, x, y, down);
                add_edge(list, x, y, right);
            } else if (x == square_length - 1 && y == square_length - 1) {
                // 右下角
                add_edge(list, x, y, up);
                add_edge(list, x, y, left);
            } else if (x == square_length - 1 && y == 0) {
                //右上角
                add_edge(list, x, y, down);
                add_edge(list, x, y, left);
            } else if (x == 0 && y == square_length - 1) {
                // 左下角
                add_edge(list, x, y, right);
                add_edge(list, x, y, up);
            } else if (x == 0) {
                //上边
                add_edge(list, x, y, down);
                add_edge(list, x, y, right);
                add_edge(list, x, y, left);
            } else if (x == square_length - 1) {
                //下边
                add_edge(list, x, y, right);
                add_edge(list, x, y, up);
                add_edge(list, x, y, left);
            } else if (y == 0) {
                //左边
                add_edge(list, x, y, down);
                add_edge(list, x, y, up);
                add_edge(list, x, y, left);
            } else if (y == square_length - 1) {
                //右边
                add_edge(list, x, y, down);
                add_edge(list, x, y, right);
                add_edge(list, x, y, up);
            } else {
                // 中间
                add_edge(list, x, y, down);
                add_edge(list, x, y, right);
                add_edge(list, x, y, up);
                add_edge(list, x, y, left);
            }
            t3->push_back(list);
        }
    }
    return t3;
}
