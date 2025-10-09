//
// Created by 潘鑫 on 2025/10/6.
//
#include <gtest/gtest.h>

#include "binary_Tree_Node.h"
#include "tree_function.h"

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

// 还需要有face

// 然后我怎么才能建立这个结构呢？
//
TEST(test_edge, test_create_edge) {
    std::vector<half_edge> half_edges;
    std::vector<vertex_xy> vertices;
    // 如果只是两个或者一些简单的边的话，其实有一个简单的办法
    // 先获取vector的size，之后添加的时候直接在size的值上增加一个或者减少一个就好。
    add_edge(half_edges, vertices, {1, 1}, {2, 2});
    add_edge(half_edges, vertices, {3, 3}, {4, 4});
    add_edge(half_edges, vertices, {0, 1.5}, {1.5, 0});
    add_edge(half_edges, vertices, {0, 5.5}, {5.5, 0});
    EXPECT_EQ(vertices.at(0).x, 1);
    EXPECT_EQ(vertices.at(0).y, 1);
    std::priority_queue<vertex_xy> copy_vertices;
    for (auto vertice: vertices) {
        copy_vertices.push(vertice);
    }
    // std::copy(vertices.begin(), vertices.end(), std::back_inserter(copy_vertices));
    // std::sort(copy_vertices.begin(), copy_vertices.end());
    // EXPECT_EQ(copy_vertices.top().x, 0);
    // 之后需要做什么呢？ // 需要将边添加到二叉树中

    binary_Tree_Node<half_edge_index> *root;
    for (; copy_vertices.empty() == false;) {
        if (copy_vertices.top().incident_half_edge < 0) {
            // 是线段中的交点,之后应该如何处理呢？
            // 问题是这应该携带什么信息？需要拿到是那两条边相交的，
            // 之后应该如何处理呢？//交换还是
        } else if (tree_find_value(root,
                                         copy_vertices.top().incident_half_edge - (
                                             copy_vertices.top().incident_half_edge % 2))
                   == nullptr) {
            root->tree_insert_value(root, copy_vertices.top().incident_half_edge);
            // 树中插入一个新的线段，判断这个线段与上一个和下一个是否相交
            // 如果相交，判断相交点与扫描线的关系，在扫描线下就将新的交点加入队列中
            // 这里会有一个新的问题，原本只需要判断是否是端点，现在还需要判断是否是线段中的交点
        } else {
            // 这里能够分辨一个线段是否已经在树中，这里是已经在树中了
            // 将目前的线段从树中删除
            // 之后呢？根据树中的索引，得到已经在树中的线段
            // 将每一个线段都与目前的线段进行是否相交的检测

            // 如果相同，那么插入到一个新的队列中 // 就是一个简单的按照区间进行比较的例子
            // 移除一个线段，判断移除之后它的上下线段是否相交
            // 如果相交，判断相交点与扫描线的关系，在扫描线下就将新的交点加入队列中
        }
        copy_vertices.pop();
        // 将边的索引插入之后，判断当前边是出边还是入边，  // 这里是线段相交的判断，并不能区分出边和入边
        // 如果是边的结束端点，那么就与在树中的全部边进行比较，
        // 得到有哪些边是相交的
        if (copy_vertices.top().incident_half_edge % 2 == 1) {
        }


        // 之后呢？之后会是两个多边形进行比较，看看哪些边是相交的
        // 需要返回什么内容呢？之需要返回相交的两条边的所以

        // 知道了两条边的索引应该做什么呢？重新建立新的多边形
        // 根据操作的不同，原本的多边形的边可能会链接到不同的边
        // 原始的两个多边形还暂时不能丢，可能会有很多的多边形执行相应的操作
    }
}
