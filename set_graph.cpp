//
// Created by 潘鑫 on 2025/2/18.
//
#include <vector>
#include <list>
#include "set_graph.h"

/**
 * 这里是逻辑，逻辑不变，但是数据的方向是可以改的
 * @param edge_list
 * @param x
 * @param y
 * @param d
 */
void add_edge(std::list<vertex_position> &edge_list, int x, int y, enum direction d) {
    if (d == up) {
        const struct vertex_position tem = {x - 1, y}; // 上
        edge_list.push_back(tem);
    } else if (d == down) {
        const struct vertex_position tem = {x + 1, y}; //下
        edge_list.push_back(tem);
    } else if (d == left) {
        const struct vertex_position tem = {x, y - 1}; // 左
        edge_list.push_back(tem);
    } else if (d == right) {
        const struct vertex_position tem = {x, y + 1}; //右
        edge_list.push_back(tem);
    }
}

void delete_edge(std::vector<vertex *> &graph, int x, int y, enum direction d) {
    auto &edge_list = graph.at(x * 10 + y)->edge_list;
    if (d == up) {
        for (const auto tem: edge_list) {
            if (tem.x_position == x - 1 && tem.y_position == y) {
                edge_list.remove(tem);
                delete_edge(graph, x - 1, y, down);
                break;
            }
        }
    } else if (d == down) {
        for (const auto tem: edge_list) {
            if (tem.x_position == x + 1 && tem.y_position == y) {
                edge_list.remove(tem);
                delete_edge(graph, x + 1, y, up);
                break;
            }
        }
    } else if (d == left) {
        for (const auto tem: edge_list) {
            if (tem.x_position == x && tem.y_position == y - 1) {
                edge_list.remove(tem);
                delete_edge(graph, x, y - 1, right);
                break;
            }
        }
    } else if (d == right) {
        for (const auto tem: edge_list) {
            if (tem.x_position == x && tem.y_position == y + 1) {
                edge_list.remove(tem);
                delete_edge(graph, x, y + 1, left);
                break;
            }
        }
    }
}


std::vector<vertex *> *init_graph(int square_length) {
    const auto t3 = new std::vector<vertex *>;
    for (int x = 0; x < square_length; ++x) {
        for (int y = 0; y < square_length; ++y) {
            const auto list = new vertex; //这里内存应该是释放了的。
            // 首先是四个角，只有两个可以到达的结点
            // 然后是四条边不包含四个角，每个有三个
            // 最后是中间的内容，都可以到达
            if (x == 0 && y == 0) {
                // 左上角
                add_edge(list->edge_list, x, y, down);
                add_edge(list->edge_list, x, y, right);
            } else if (x == square_length - 1 && y == square_length - 1) {
                // 右下角
                add_edge(list->edge_list, x, y, up);
                add_edge(list->edge_list, x, y, left);
            } else if (x == square_length - 1 && y == 0) {
                //右上角
                add_edge(list->edge_list, x, y, down);
                add_edge(list->edge_list, x, y, left);
            } else if (x == 0 && y == square_length - 1) {
                // 左下角
                add_edge(list->edge_list, x, y, right);
                add_edge(list->edge_list, x, y, up);
            } else if (x == 0) {
                //上边
                add_edge(list->edge_list, x, y, down);
                add_edge(list->edge_list, x, y, right);
                add_edge(list->edge_list, x, y, left);
            } else if (x == square_length - 1) {
                //下边
                add_edge(list->edge_list, x, y, right);
                add_edge(list->edge_list, x, y, up);
                add_edge(list->edge_list, x, y, left);
            } else if (y == 0) {
                //左边
                add_edge(list->edge_list, x, y, down);
                add_edge(list->edge_list, x, y, up);
                add_edge(list->edge_list, x, y, left);
            } else if (y == square_length - 1) {
                //右边
                add_edge(list->edge_list, x, y, down);
                add_edge(list->edge_list, x, y, right);
                add_edge(list->edge_list, x, y, up);
            } else {
                // 中间
                add_edge(list->edge_list, x, y, down);
                add_edge(list->edge_list, x, y, right);
                add_edge(list->edge_list, x, y, up);
                add_edge(list->edge_list, x, y, left);
            }
            t3->push_back(list);
        }
    }
    return t3;
}

bool delete_graph(const std::vector<vertex *> &graph) {
    for (const auto &it: graph) {
        delete it;
    }
    return true;
}

/*寻路算法应该怎么写，它的逻辑是什么样子的，
 *首先把起始点添加到队列
 *然后把队列中最小的点取出，直到队列为空为止
 * 遍历最小的点的链表中所有能够到达的点，
 *          如果想提前停止，那么就需要添加一个if
 *          到达终止节点的时候退出这个循环
 *    计算每个点到起始点到距离，
 *    然后把这个距离值，连同是哪个点这两个参数添加到队列中或者在队列中修改
 *    并且更新上一个当前结点的最短路径的上一个点
 *
 *    然后就是想要输出的话，就需要从终止点向前遍历，找到相应的路径。
 *
 * 还有一个算法，起始点位置和终止点位置是知道的，那么可以由两个点之间的距离来当队列中的排序方式
 * 也就是权重和最小是一种方式，理想距离最小也是一种方式
 * 上面两个和起来的最小值，也是一种方式。
 * 看了看书上的算法，又发现了我这里的问题，
 * 书上是在最开始就把全部的点都加入了队列中，还用另一个集合来保存已经是最小路径的点
 *
 * 我这里会有重复添加点的可能，只有是有向无环图的时候才不会重复添加点
 * 书上的更好，先按照书上来吧，
 * 但是最开始把全部点都加入队列中时，不能更新理想距离，
 * 必须从起始点开始，如果不从起始点开始，？？没想明白
 * 开始改造现有的代码吧，关于距离好像是三个距离，
 * 直线距离（欧式距离，欧里几得，几何学之父），
 * 曼哈顿距离， 曼哈顿距离是有四个方向，沿每个方向行走单位距离，距离加一
 * 切尔雪夫距离，  切尔雪夫距离是有八个方向，沿每个方向到达另一个格子，距离增加一
 *
 * 现在先按照曼哈顿距离来算法。等稍微改造一下结构题
 * 怎么改造呢？每个点增加一个存储距离的元素，再增加一个上一个点是指向哪里的元素
 * 那么每个点其实都是需要一个结构体的，结构体中，是否需要保护当前自己的位置呢？需要的
 *
 *
 *
 *
 *
 *
 *
 *
*/
