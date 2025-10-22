//
// Created by 潘鑫 on 2025/10/21.
//

#ifndef HALF_EDGE_H
#define HALF_EDGE_H
#include <vector>
#include "vector_signed_area.h"
//
// Created by 潘鑫 on 2025/10/21.
//

struct vertex_xy {
    float x, y;
    int incident_half_edge;
    int is_using; // 暂时没有办法的一个办法了 // 用于判断是否当前端点或者其他是否有在使用
    bool operator<(const vertex_xy &right) const {
        if (y < right.y) {
            // 先比较x轴，x轴小的为小
            return true;
        } else if (y == right.y && x < right.x) {
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
    half_edge_index bounding_half_edge;

    enum BOUNDARY_TYPE {
        bounding_face,
        hole_face,
    };

    BOUNDARY_TYPE boundary_type;
};


// void add_edge(std::vector<half_edge> &half_edges, std::vector<vertex_xy> &vertices, vertex_xy start_point,
//               vertex_xy end_point) {
//     int half_edges_size = half_edges.size();
//     int vertices_size = vertices.size();
//     vertices.push_back(start_point);
//     vertices.push_back(end_point);
//     half_edges.push_back({vertices_size, vertices_size + 1, 0});
//     half_edges.push_back({vertices_size + 1, vertices_size, 0});
// }


// 还需要有face
struct half_edge_struct {
    std::vector<half_edge> half_edges;
    std::vector<vertex_xy> vertices;
    std::vector<face> faces;

    half_edge_index add_edge(vertex_xy start_point,
                             vertex_xy end_point) {
        int half_edges_size = half_edges.size();
        int vertices_size = vertices.size();
        start_point.incident_half_edge = half_edges_size;
        end_point.incident_half_edge = half_edges_size + 1;
        vertices.push_back(start_point);
        vertices.push_back(end_point);
        half_edges.push_back({vertices_size, half_edges_size + 1, 0});
        // 上面这条边插入的是什么呢？它的vertex 和 边是一一对应的，知道一条边，能够知道它的起点
        half_edges.push_back({vertices_size + 1, half_edges_size, 0});
        return half_edges_size;
    }

    // 之后再在内部的面上增加顶点，然后就有一个问题，
    // 如果之前内部的面是空的话，那么不需要创建新的face
    // 如果不为空的话，那么是什么样子的呢？
    // 需要新建立一个面，并更改掉一些原本的内容
    half_edge_index add_edge(half_edge_index current_edge, vertex_xy middle_point) {
        int half_edges_size = half_edges.size();
        int vertices_size = vertices.size();
        int faces_size = faces.size();
        int middle_point_index = vertices_size + 1;
        vertices.push_back(middle_point);
        // 单纯的加点很好加

        int vertex_index_start_point = half_edges.at(current_edge).vertex_index;
        int vertex_index_end_point = half_edges.at(get_opposite(current_edge)).vertex_index;
        int black_a = get_opposite(current_edge);
        int red_b = current_edge;
        int red_c = half_edges_size;
        int blue_d = half_edges_size + 1;
        int red_e = half_edges_size + 2;
        int blue_f = half_edges_size + 3;

        half_edges.push_back({
            vertex_index_start_point, blue_d,
            red_e, red_b, faces_size
        });
        half_edges.push_back({
            middle_point_index, half_edges_size,
            get_next(red_b), blue_f, faces_size
        });
        half_edges.push_back({
            middle_point_index, blue_f,
            red_b, red_c, faces_size
        });
        half_edges.push_back({
            vertex_index_end_point, red_e,
            blue_d, get_pre(red_b), faces_size
        });
        // 添加新的四条边也是能够添加的
        // 问题是原本的边应该怎么动？
        // 判断一下get_next(get_opposite(current_edge)) == current_edge
        // 不需要，直接执行就好，能够直接满足两种条件
        half_edges.at(get_next(red_b)).pre_half_edge = blue_d;
        half_edges.at(get_pre(red_b)).next_half_edge = blue_f;
        half_edges.at(red_b).next_half_edge = red_c;
        half_edges.at(red_b).pre_half_edge = red_e;
        faces.push_back({red_c, face::BOUNDARY_TYPE::bounding_face});
        return red_c;
    }

    // 如果想将原本的half_edge 中，添加一个顶点，将它分为两个edge的操作
    half_edge_index split_edge(half_edge_index current_edge, vertex_xy middle_point) {
        int half_edges_size = half_edges.size();
        int vertices_size = vertices.size();
        int faces_size = faces.size();
        int middle_point_index = vertices_size;
        vertices.push_back(middle_point);
        // 单纯的加点很好加

        int blue_a = get_opposite(current_edge);
        int green_b = current_edge;
        int green_c = get_next(green_b);
        int blue_d = get_pre(blue_a);
        int blue_e = half_edges_size + 1;
        int green_f = half_edges_size;

        int vertex_index_start_point = half_edges.at(blue_a).vertex_index;
        int vertex_index_end_point = half_edges.at(green_c).vertex_index;

        // 这里需要做一个判断
        if (get_next(get_opposite(current_edge)) == current_edge) {
            half_edges.push_back({
                middle_point_index, blue_e,
                blue_e, green_b, faces_size
            });
            half_edges.push_back({
                vertex_index_end_point, green_f,
                blue_a, green_f, faces_size
            });
            half_edges.at(green_b).next_half_edge = green_f;
            half_edges.at(blue_a).pre_half_edge = blue_e;
        } else {
            half_edges.push_back({
                middle_point_index, blue_e,
                green_c, green_b, faces_size
            });
            half_edges.push_back({
                vertex_index_end_point, green_f,
                blue_a, blue_d, faces_size
            });
            half_edges.at(green_c).pre_half_edge = green_f;
            half_edges.at(blue_d).next_half_edge = blue_e;
            half_edges.at(green_b).next_half_edge = green_f;
            half_edges.at(blue_a).pre_half_edge = blue_e;
        }
        return green_f;
    }

    // 两个顶点创建一个loop，然后后创建两个面，一个是内部的面，另一个是外部的面，
    // 创建loop的时候只会创建一个面。，这个退化的线之内全部都是这个面
    half_edge_index create_loop(vertex_xy start_point,
                                vertex_xy end_point) {
        int half_edges_size = half_edges.size();
        int vertices_size = vertices.size();
        int faces_size = faces.size();
        start_point.incident_half_edge = half_edges_size;
        end_point.incident_half_edge = half_edges_size + 1;
        vertices.push_back(start_point);
        vertices.push_back(end_point);
        half_edges.push_back({
            vertices_size, half_edges_size + 1,
            half_edges_size + 1, half_edges_size + 1, faces_size
        });
        half_edges.push_back({
            vertices_size + 1, half_edges_size,
            half_edges_size, half_edges_size, faces_size
        });
        faces.push_back({half_edges_size, face::BOUNDARY_TYPE::hole_face});
        return half_edges_size;
    }


    segment_position get_segment(int incident_half_edge) {
        // 稍微有一点点的问题啊？
        int vertex_index_end_point = half_edges.at(incident_half_edge).vertex_index;
        point_2 start_point{
            vertices.at(vertex_index_end_point).x,
            vertices.at(vertex_index_end_point).y
        };
        int twin_half_edge = half_edges.at(incident_half_edge).twin_half_edge;
        int vertex_index_start_point = half_edges.at(twin_half_edge).vertex_index;
        point_2 end_point{
            vertices.at(vertex_index_start_point).x,
            vertices.at(vertex_index_start_point).y
        };
        segment_position result{start_point, end_point};
        return result;
    }

    int get_same_edge_index(int incident_half_edge) {
        return incident_half_edge - (incident_half_edge % 2);
    }

    vertex_xy &get_vertex_xy(int incident_half_edge) {
        int same_edge_index = get_same_edge_index(incident_half_edge);
        int vertex_index_end_point = half_edges.at(same_edge_index).vertex_index;
        return vertices.at(vertex_index_end_point);
    }

    [[nodiscard]] half_edge_index get_next(const int incident_half_edge) const {
        const int result = half_edges.at(incident_half_edge).next_half_edge;
        return result;
    }

    [[nodiscard]] half_edge_index get_pre(const int incident_half_edge) const {
        const int result = half_edges.at(incident_half_edge).pre_half_edge;
        return result;
    }

    [[nodiscard]] half_edge_index get_twin(const int incident_half_edge) const {
        const int result = half_edges.at(incident_half_edge).twin_half_edge;
        return result;
    }

    [[nodiscard]] half_edge_index get_opposite(const int incident_half_edge) const {
        const int result = half_edges.at(incident_half_edge).twin_half_edge;
        return result;
    }

    std::vector<half_edge_index> get_all_edge_of_vertex(int vertex_index) {
        std::vector<half_edge_index> result;
        int current_half_edge_index = vertices.at(vertex_index).incident_half_edge;
        int first_half_edge = current_half_edge_index;
        while (true) {
            auto opposite_edge = get_opposite(current_half_edge_index);
            auto next_half_edge = get_next(current_half_edge_index);
            result.push_back(next_half_edge);
            current_half_edge_index = next_half_edge;
            if (current_half_edge_index != first_half_edge) {
                return result;
            }
        }
    }

    std::vector<half_edge_index> get_all_edge_of_face(face face) {
        std::vector<half_edge_index> result;
        int current_half_edge_index = face.bounding_half_edge;
        int first_half_edge = current_half_edge_index;
        while (true) {
            auto next_half_edge = get_next(current_half_edge_index);
            result.push_back(next_half_edge);
            current_half_edge_index = next_half_edge;
            if (current_half_edge_index != first_half_edge) {
                return result;
            }
        }
    }
};


#endif //HALF_EDGE_H
