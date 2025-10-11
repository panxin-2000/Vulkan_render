//
// Created by 潘鑫 on 2025/10/6.
//
#include <gtest/gtest.h>

#include "binary_Tree_Node.h"
#include "tree_function.h"
#include "vector_signed_area.h"

// 既然我们这里需要使用plane sweep 算法，
// 那么先说说这个算法是怎么实现的呢？
// 需要首先需要一系列的边，
// 之后才能判断两条边是否相交，
// 然后需要什么呢？需要将全部的边的端点排序好之后添加到一个向量中
// 之后，从这个向量中，不断的拿出端点所对应的边
// 将边添加到树中，每次添加一个边，
// 问题是在树中是按照什么来进行排列的呢？
// 在向量中是按照x轴来排列的，那么到树中的话，是需要按照y轴来排列的
// 左端点是添加到树中，右端点是从树中移出，如果右相同的x值，那么左端点要在右端点之前
//
// 我似乎有一个记录，之前的时候应该手画过这个过程
// 看书的时候发现了一个新的不一样的地方，就是需要判断两条线谁在谁的上方

// 我们可以先这样做，判断右端点离开时是否和其他任何一条线相交，这个是一个最简单的办法
// 因为只是按照x轴的区间进行分类

// 之后需要做更负责的内容，
// 怎么才能判断是否出现了变化呢？
// 新的端点进来的时候需要添加什么样的内容？
// 至少需要先做出一个结果，之后再更改
//

struct vertex_xy {
    float x, y;
    int incident_half_edge;
    int is_using; // 暂时没有办法的一个办法了 // 用于判断是否当前端点或者其他是否有在使用
    bool operator<(const vertex_xy &right) const {
        if (x < right.x) {
            // 先比较x轴，x轴小的为小
            return true;
        } else if (x == right.x && y < right.y) {
            // 之后再比较y轴，y轴小的为小
            return true;
        } else if (x == right.x && y == right.y) {
            if (incident_half_edge % 2 == 0) {
                // 值完全一样，比较是否是起点，是起点的边，
                return true; // a起点，b不是起点，a小，a不是起点，那么b是不是起点都在a前，没什么关系
            }
        }
        return false;
    }

    bool operator==(const vertex_xy &right) const {
        if (x == right.x && y == right.y && incident_half_edge == right.incident_half_edge) {
            return true;
        }
        return false;
    }
};


struct event_point {
    float x, y;
    int incident_half_edge;
    int intersect_half_edge_1;
    int intersect_half_edge_2;
    int if_intersect; // 暂时没有办法的一个办法了 // 用于判断是否当前端点或者其他是否有在使用
    bool operator<(const event_point &right) const {
        if (x < right.x) {
            // 先比较x轴，x轴小的为小
            return true;
        } else if (x == right.x && y < right.y) {
            // 之后再比较y轴，y轴小的为小
            return true;
        } else if (x == right.x && y == right.y) {
            if (incident_half_edge % 2 == 0) {
                // 值完全一样，比较是否是起点，是起点的边，
                return true; // a起点，b不是起点，a小，a不是起点，那么b是不是起点都在a前，没什么关系
            }
        }
        return false;
    }

    bool operator>(const event_point &right) const {
        return !this->operator<(right);
    }
};


struct half_edge {
    int vertex_index;
    int twin_half_edge;
    int next_half_edge;
    int pre_half_edge;
    int incident_face;
    int is_using;
};

using half_edge_index = int;
// 索引还是比较啊随意的，问题是如何建立一条边？

struct face {
    std::vector<half_edge_index> bounding_half_edge;
    std::vector<half_edge_index> hole_half_edge;
};

void add_edge(std::vector<half_edge> &half_edges, std::vector<vertex_xy> &vertices, vertex_xy start_point,
              vertex_xy end_point) {
    int half_edges_size = half_edges.size();
    int vertices_size = vertices.size();
    vertices.push_back(start_point);
    vertices.push_back(end_point);
    half_edges.push_back({vertices_size, vertices_size + 1, 0});
    half_edges.push_back({vertices_size + 1, vertices_size, 0});
}

int get_same_edge_index(int incident_half_edge) {
    return incident_half_edge - (incident_half_edge % 2);
}

bool if_half_edge_in_tree(binary_Tree_Node<half_edge_index> *root, int incident_half_edge) {
    if (tree_find_value(root, get_same_edge_index(incident_half_edge))
        == nullptr) {
        return false;
    } else {
        return true;
    }
}

// 还需要有face
struct half_edge_struct {
    std::vector<half_edge> half_edges;
    std::vector<vertex_xy> vertices;

    void add_edge(vertex_xy start_point,
                  vertex_xy end_point) {
        int half_edges_size = half_edges.size();
        int vertices_size = vertices.size();
        start_point.incident_half_edge = half_edges_size;
        end_point.incident_half_edge = half_edges_size + 1;
        vertices.push_back(start_point);
        vertices.push_back(end_point);
        half_edges.push_back({vertices_size, vertices_size + 1, 0});
        half_edges.push_back({vertices_size + 1, vertices_size, 0});
    }

    segment_position get_segment_from_node(int incident_half_edge) {
        // 稍微有一点点的问题啊？
        int vertex_index_end_point = half_edges.at(incident_half_edge).vertex_index;
        segment_vector end_point{
            vertices.at(vertex_index_end_point).x,
            vertices.at(vertex_index_end_point).y
        };
        int twin_half_edge = half_edges.at(incident_half_edge).twin_half_edge;
        int vertex_index_start_point = half_edges.at(twin_half_edge).vertex_index;
        segment_vector start_point{
            vertices.at(vertex_index_start_point).x,
            vertices.at(vertex_index_start_point).y
        };
        segment_position result{start_point, end_point};
        return result;
    }
};

// 然后我怎么才能建立这个结构呢？
//  其实应该先写一个最简单暴力的来，不然不太好玩
//
TEST(test_edge, test_create_edge) {
    half_edge_struct hf;
    hf.add_edge({1, 1}, {2, 2});
    hf.add_edge({3, 3}, {4, 4});
    hf.add_edge({0, 2}, {2, 0});
    hf.add_edge({0, 6.5}, {6.5, 0});

    EXPECT_EQ(hf.vertices.at(0).x, 1);
    EXPECT_EQ(hf.vertices.at(0).y, 1);
    // std::less 这个操作还是很有用的，知道它本身调用了什么更好
    std::priority_queue<event_point, std::vector<event_point>, std::greater<event_point> > event_points;
    for (auto vertice: hf.vertices) {
        event_point temp;
        temp.x = vertice.x;
        temp.y = vertice.y;
        temp.incident_half_edge = vertice.incident_half_edge;
        temp.intersect_half_edge_1 = -1;
        temp.intersect_half_edge_2 = -1;
        temp.if_intersect = 0;
        event_points.push(temp);
    }
    binary_Tree_Node<half_edge_index> *root;
    for (; event_points.empty() == false;) {
        auto current_half_edge = event_points.top().incident_half_edge;
        if (event_points.top().if_intersect == true) {
            // 是线段中的交点,之后应该如何处理呢？
            // 问题是这应该携带什么信息？需要拿到是那两条边相交的，
            // 之后应该如何处理呢？//交换,既然是相交的，那么他们之前一定是相邻的，交互两个结点就好
        } else if (if_half_edge_in_tree(root, current_half_edge) == false) {
            root = root->tree_insert_value(root, current_half_edge);
            auto current_half_edge_node = tree_find_value(root, current_half_edge);
            auto predecessor_half_edge_node = current_half_edge_node->tree_predecessor(current_half_edge_node);
            auto successor_half_edge_node = current_half_edge_node->tree_successor(current_half_edge_node);
            if (current_half_edge_node != nullptr && predecessor_half_edge_node != nullptr) {
                // 两条线段判断是否相交
                segment_position ab = hf.get_segment_from_node(current_half_edge_node->data);
                segment_position cd = hf.get_segment_from_node(predecessor_half_edge_node->data);

                if (ab.intersection(cd) == true) {
                    // 如果相交，把交点插入到事件点中，并且需要判断交点是否在扫描线之后
                    segment_vector result;
                    if (ab.get_intersection_point(cd, &result) == true) {
                        std::cout << " intersect point" << result.x << "  " << result.y << std::endl;
                    }
                }
            }
            if (current_half_edge_node != nullptr && successor_half_edge_node != nullptr) {
                // 两条线段判断是否相交
                segment_position ab = hf.get_segment_from_node(current_half_edge_node->data);
                segment_position cd = hf.get_segment_from_node(successor_half_edge_node->data); // 这有问题，导致了死机
                if (ab.intersection(cd) == true) {
                    // 如果相交，把交点插入到事件点中，并且需要判断交点是否在扫描线之后
                    segment_vector result;
                    if (ab.get_intersection_point(cd, &result) == true) {
                        std::cout << " intersect point " << result.x << "  " << result.y << std::endl;
                    }
                }
            }
            // 树中插入一个新的线段，判断这个线段与上一个和下一个是否相交
            // 如果相交，判断相交点与扫描线的关系，在扫描线下就将新的交点加入队列中
            // 这里会有一个新的问题，原本只需要判断是否是端点，现在还需要判断是否是线段中的交点
        } else {
            auto delate_node = tree_find_value(root, get_same_edge_index(current_half_edge));
            root = root->delete_node_from_binary_search_tree(root, *delate_node);


            // 这里能够分辨一个线段是否已经在树中，这里是已经在树中了
            // 将目前的线段从树中删除
            // 之后呢？根据树中的索引，得到已经在树中的线段
            // 将每一个线段都与目前的线段进行是否相交的检测

            // 如果相同，那么插入到一个新的队列中 // 就是一个简单的按照区间进行比较的例子
            // 移除一个线段，判断移除之后它的上下线段是否相交
            // 如果相交，判断相交点与扫描线的关系，在扫描线下就将新的交点加入队列中
        }
        event_points.pop();
        // 将边的索引插入之后，判断当前边是出边还是入边，  // 这里是线段相交的判断，并不能区分出边和入边
        // 如果是边的结束端点，那么就与在树中的全部边进行比较，
        // 得到有哪些边是相交的
        if (event_points.top().incident_half_edge % 2 == 1) {
        }


        // 之后呢？之后会是两个多边形进行比较，看看哪些边是相交的
        // 需要返回什么内容呢？之需要返回相交的两条边的所以

        // 知道了两条边的索引应该做什么呢？重新建立新的多边形
        // 根据操作的不同，原本的多边形的边可能会链接到不同的边
        // 原始的两个多边形还暂时不能丢，可能会有很多的多边形执行相应的操作
    }
}
