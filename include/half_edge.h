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
    };;
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

    segment_position get_segment(int incident_half_edge) {
        // 稍微有一点点的问题啊？
        int vertex_index_end_point = half_edges.at(incident_half_edge).vertex_index;
        segment_vector start_point{
            vertices.at(vertex_index_end_point).x,
            vertices.at(vertex_index_end_point).y
        };
        int twin_half_edge = half_edges.at(incident_half_edge).twin_half_edge;
        int vertex_index_start_point = half_edges.at(twin_half_edge).vertex_index;
        segment_vector end_point{
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
