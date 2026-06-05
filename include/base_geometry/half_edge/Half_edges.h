//
// Created by 潘鑫 on 2025/10/21.
//

#ifndef HALF_EDGE_H
#define HALF_EDGE_H
#include <vector>

#include "base_geometry/base.h"
#include "base_geometry/convex_hull.h"


//
// Created by 潘鑫 on 2025/10/21.
//

#include "../../point_in_on_out_triangle.h"
#include "../../triangle_graph.h"


// 索引还是比较啊随意的，问题是如何建立一条边？
#include "base_geometry/half_edge/half_edge.h"
#include "base_geometry/half_edge/face.h"


// 还可以把half_edge和face也做出模版参数,方便进行扩展
template<typename Vertex>
struct Half_edges {
    std::vector<Half_edge> edges;
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    std::vector<Face_index> faces_delete;
    using vertex_base_type = typename Vertex::point_type;

    struct History {
        std::vector<std::pair<Vertex_index, Vertex> > vertices;
        std::vector<std::pair<Half_edge_index, Half_edge> > half_edges;
        std::vector<std::pair<Face_index, Face> > faces;
    };

    std::array<History, 10> history;
    uint16_t current_history = 0;

    // 还需要添加检查，不要造成重复添加值
#define half_edges_change(index,detail)                            \
    if constexpr(false){                                           \
    auto &temp = history[current_history].edges;                   \
    temp.push_back(std::make_pair(index, edges.at(index)));        \
    }                                                              \
    edges.at(index).detail

    // half_edge_index add_edge(Vertex start_point, Vertex end_point) {
    //     auto half_edges_size           = edges.size();
    //     const auto vertices_size       = vertices.size();
    //     start_point.incident_half_edge = half_edges_size;
    //     end_point.incident_half_edge   = half_edges_size + 1;
    //     vertices.push_back(start_point);
    //     vertices.push_back(end_point);
    //     add_half_edge(vertices_size, half_edges_size + 1, 0, 0);
    //     // 上面这条边插入的是什么呢？它的vertex 和 边是一一对应的，知道一条边，能够知道它的起点
    //     add_half_edge(vertices_size + 1, half_edges_size, 0, 0);
    //     return half_edges_size;
    // }


private:
    Half_edge_index add_half_edge(const Vertex_index vertex_index,
                                  const Half_edge_index temp_twin_half_edge,
                                  const Half_edge_index next_half_edge,
                                  const Half_edge_index pre_half_edge,
                                  const Face_index incident_face) {
        return add_half_edge(vertex_index, next_half_edge, pre_half_edge, incident_face);
    }

    Half_edge_index add_half_edge(const Vertex_index vertex_index,
                                  const Half_edge_index next_half_edge,
                                  const Half_edge_index pre_half_edge,
                                  const Face_index incident_face) {
        const auto half_edges_size = edges.size();
        Half_edge_index twin_half_edge;
        // half_edges_size 为 0 时， twin_half_edge = 1
        // half_edges_size 为 1 时， twin_half_edge = 0
        if (half_edges_size % 2 == 0) {
            twin_half_edge = half_edges_size + 1;
        } else {
            twin_half_edge = half_edges_size - 1;
        }
        Half_edge temp{};
        temp.next_half_edge = next_half_edge;
        temp.pre_half_edge  = pre_half_edge;
        temp.incident_face  = incident_face;
        temp.twin_half_edge = twin_half_edge;
        temp.vertex_index   = vertex_index;


        edges.push_back(temp);
        return half_edges_size;
    }

public:
    // 将一个 vertex 连接到 edge 的两个端点
    Half_edge_index add_edge(Half_edge_index current_edge, Vertex middle_point);


    bool delete_edge(Half_edge_index current_edge);


    /**
     * 在AB边上添加一个点，现在这个边变成了ADB，返回DB这条边
     * before: E-----A--------------B------C
     * end   : E-----A------D-------B------C
     * @param edge_AB
     * @param point_D
     * @return
     */
    Half_edge_index insert_edge(Half_edge_index edge_AB, Vertex point_D);

    Half_edge_index add_triangle(Vertex point_a, Vertex point_b, Vertex point_c) {
        auto half_edge_index_s = add_edge(point_b, point_a);
        auto first_half_edge   = half_edge_index_s;
        add_edge(half_edge_index_s, point_c);
        return half_edge_index_s;
    }


    // 两个顶点创建一个loop，然后后创建两个面，一个是内部的面，另一个是外部的面，
    // 只会创建一个面。，这个退化的线之内全部都是这个面
    // 返回 start_point 指向 end_point 索引的边，
    // 假设输入是 a , b , 那么返回的是 ab 的索引
    Half_edge_index add_edge(Vertex start_point,
                             Vertex end_point);

    // 在一个两个顶点直接插入一条边，将原本的一个face，分为两个face
    Half_edge_index split_face(Face_index split_face, Vertex first_point,
                               Vertex second_point);

    Half_edge_index split_face(Half_edge_index first_edge,
                               Vertex second_point);

    // 最后开始做的时候没有写注释
    // 参考上面的代码，才能知道具体的内容是什么
    Half_edge_index split_face(Half_edge_index first_edge,
                               Half_edge_index second_edge);

    Half_edge_index
    get_ab_edge_from_face_abc(const Face_index face_index_para, const Vertex_index vertex_c_index) {
        const auto edge_indices = get_all_edge_of_face(get_face(face_index_para).bounding_half_edge);
        for (const auto edge_index: edge_indices) {
            if (get_edge(edge_index).vertex_index != vertex_c_index ||
                get_edge(get_opposite_edge_index(edge_index)).vertex_index != vertex_c_index) {
                return edge_index;
            }
        }
    }

    /**
     * 给一个三角形的face中间添加一个点，变更为三个face
     * @param face_index_para
     * @param add_point
     * @param vertex_index
     * @return 返回这三个face的索引
     */
    std::vector<Face_index> face_add_inner_point(Face_index face_index_para, Vertex add_point,
                                                 Vertex_index &vertex_index);


    /***************************下面的代码都不会更改原本的结构***********************************/
    /**
     * 一个边会有两个相邻的面，判断这两个相邻的面组成的多边形是否是凸的
     * @param edge_index
     * @return
     */
    bool if_convex_quadrangle(const Half_edge_index edge_index) {
        const auto opposite_edge_index      = get_opposite_edge_index(edge_index);
        const auto all_edges_of_first_face  = get_all_edge_of_face(edge_index);
        const auto all_edges_of_second_face = get_all_edge_of_face(opposite_edge_index);
        if (all_edges_of_first_face.size() == 3 && all_edges_of_second_face.size() == 3) {
            // 这时候就可以拿到四个点了
            const Trapezoid temp{
                get_vertex(get_pre_edge_index(edge_index)),
                get_vertex(edge_index),
                get_vertex(get_pre_edge_index(opposite_edge_index)),
                get_vertex(opposite_edge_index)
            };

            if (convex(temp)) {
                return true;
            }
        }
        return false;
    }


    /**
     * 给出两个三角形，ABC和CBD，更改为三角形ABD和DCA
     * @param bc_or_cb_edge_index
     * @return
     */
    bool flip_edge(Half_edge_index bc_or_cb_edge_index);

    bool set_face_for_edge_loop(const Half_edge_index half_edge_index_of_face, const Face_index faces_index) {
        auto faces_size              = faces_index;
        auto current_half_edge_index = half_edge_index_of_face;
        auto first_half_edge         = current_half_edge_index;
        while (true) {
            edges.at(current_half_edge_index).incident_face = faces_size;
            auto next_half_edge                             = get_next_edge_index(current_half_edge_index);
            current_half_edge_index                         = next_half_edge;
            if (current_half_edge_index == first_half_edge) {
                return true;
            }
        }
        return false;
    }


    Segment<vertex_base_type> get_segment(const Half_edge_index incident_half_edge) {
        // 稍微有一点点的问题啊？
        const auto twin_half_edge           = edges.at(incident_half_edge).twin_half_edge;
        const auto vertex_index_start_point = edges.at(incident_half_edge).vertex_index;
        const auto vertex_index_end_point   = edges.at(twin_half_edge).vertex_index;

        vertex_base_type start_point{
            vertices.at(vertex_index_start_point).x,
            vertices.at(vertex_index_start_point).y
        };
        vertex_base_type end_point{
            vertices.at(vertex_index_end_point).x,
            vertices.at(vertex_index_end_point).y
        };
        Segment<vertex_base_type> result{start_point, end_point};
        return result;
    }

    std::vector<Segment<> > get_all_segments() {
        std::vector<Segment<> > result;
        for (size_t i = 0; i < edges.size(); ++i, ++i) {
            auto segment = get_segment(i);
            result.emplace_back(segment);
        }
        return result;
    }

    Half_edge_index get_same_edge_index(const Half_edge_index incident_half_edge) {
        return incident_half_edge - (incident_half_edge % 2);
    }

    Vertex &get_vertex(const Half_edge_index incident_half_edge) {
        auto vertex_index_end_point = edges.at(incident_half_edge).vertex_index;
        return vertices.at(vertex_index_end_point);
    }


    Half_edge &get_edge(Half_edge_index incident_half_edge) {
        return edges.at(incident_half_edge);
    }

    Face &get_face_with_one_edge(Half_edge_index incident_half_edge) {
        return faces.at(get_edge(incident_half_edge).incident_face);
    }

    Face &get_face(const Face_index face) {
        return faces.at(face);
    }

    [[nodiscard]] Half_edge_index &get_face_incident_edge(const Face_index face_index) {
        return faces.at(face_index).bounding_half_edge;
    }

    [[nodiscard]] Half_edge_index get_next_edge_index(const Half_edge_index half_edge_index) const {
        const auto result = edges.at(half_edge_index).next_half_edge;
        return result;
    }

    [[nodiscard]] Half_edge_index get_pre_edge_index(const Half_edge_index half_edge_index) const {
        const auto result = edges.at(half_edge_index).pre_half_edge;
        return result;
    }

    [[nodiscard]] Half_edge_index get_twin_edge_index(const Half_edge_index half_edge_index) const {
        const auto result = edges.at(half_edge_index).twin_half_edge;
        return result;
    }

    [[nodiscard]] Half_edge_index get_opposite_edge_index(const Half_edge_index half_edge_index) const {
        const auto result = edges.at(half_edge_index).twin_half_edge;
        return result;
    }

    [[nodiscard]] Vertex_index get_vertices_index(const Half_edge_index half_edge_index) const {
        const auto result = edges.at(half_edge_index).vertex_index;
        return result;
    }

    [[nodiscard]] Face_index get_face_index(Half_edge_index half_edge_index) {
        return edges.at(half_edge_index).incident_face;
    }

    std::vector<Half_edge_index> get_all_edge_of_vertex(Vertex_index vertex_index) {
        std::vector<Half_edge_index> result;
        auto current_half_edge_index = vertices.at(vertex_index).incident_half_edge;
        auto first_half_edge         = current_half_edge_index;
        while (true) {
            auto opposite_edge = get_opposite_edge_index(current_half_edge_index);
            result.push_back(opposite_edge);
            auto next_half_edge     = get_next_edge_index(opposite_edge);
            current_half_edge_index = next_half_edge;
            if (current_half_edge_index == first_half_edge) {
                return result;
            }
        }
    }

    std::vector<Face_index> get_all_face_of_vertex(Vertex_index vertex_index) {
        auto all_edges = get_all_edge_of_vertex(vertex_index);
        std::vector<Face_index> result;
        for (Half_edge_index single_edge: all_edges) {
            result.push_back(get_edge(single_edge).incident_face);
        }
        return result;
    }

    std::vector<Half_edge_index> get_all_edge_of_face(const Half_edge_index half_edge_index_of_face) {
        std::vector<Half_edge_index> result;
        auto current_half_edge_index = half_edge_index_of_face;
        auto first_half_edge         = current_half_edge_index;
        while (true) {
            auto next_half_edge = get_next_edge_index(current_half_edge_index);
            result.push_back(next_half_edge);
            current_half_edge_index = next_half_edge;
            if (current_half_edge_index == first_half_edge) {
                return result;
            }
        }
    }

    bool get_first_face(Face &int_put_face) {
        for (auto face: faces) {
            if (face.boundary_type != Face::BOUNDARY_TYPE::hole_face) {
                int_put_face.bounding_half_edge = face.bounding_half_edge;
                int_put_face.boundary_type      = face.boundary_type;
                return true;
            }
        }
        return false;
    }


    [[nodiscard]] std::vector<Point_2> get_vertices(const std::vector<Half_edge_index> &half_edge_indices) const {
        std::vector<Point_2> points{};
        for (auto half_edge_index: half_edge_indices) {
            Point_2 temp_point{};
            temp_point.x = vertices.at(edges.at(half_edge_index).vertex_index).x;
            temp_point.y = vertices.at(edges.at(half_edge_index).vertex_index).y;
            points.push_back(temp_point);
        }
        return points;
    }

    /**
     *
     * @tparam T
     * @param without_hole true 时 不输出洞， false 输出洞
     * @return
     */
    auto get_all_triangles_data(bool without_hole) {
        std::vector<Triangle<vertex_base_type> > result_triangles;
        for (auto face: faces) {
            if (!without_hole || face.boundary_type != Face::BOUNDARY_TYPE::hole_face) {
                auto temp = get_all_edge_of_face(face.bounding_half_edge);
                if (temp.size() == 3) {
                    auto a = get_vertex(temp.at(0));
                    auto b = get_vertex(temp.at(1));
                    auto c = get_vertex(temp.at(2));

                    Triangle<vertex_base_type> t{a, b, c};
                    result_triangles.push_back(t);
                }
            }
        }
        return result_triangles;
    }

    template<typename T>
    bool get_triangle_vertex_index(T &result_triangles, bool without_hole) {
        for (auto face: faces) {
            if (!without_hole || face.boundary_type != Face::BOUNDARY_TYPE::hole_face) {
                auto temp = get_all_edge_of_face(face.bounding_half_edge);
                if (temp.size() == 3) {
                    auto a = get_vertices_index(temp.at(0));
                    auto b = get_vertices_index(temp.at(1));
                    auto c = get_vertices_index(temp.at(2));
                    result_triangles.push_back(a);
                    result_triangles.push_back(b);
                    result_triangles.push_back(c);
                } else {
                    return false;
                }
            }
        }
        return true;
    }

    std::vector<vertex_base_type> &get_vertices_vector() {
        return vertices;
    }

    Triangle<vertex_base_type> get_triangle_face_vertex(const Face_index face_index_para) {
        auto temps = get_all_edge_of_face(get_face_incident_edge(face_index_para));
        auto a     = get_vertex(temps.at(0));
        auto b     = get_vertex(temps.at(1));
        auto c     = get_vertex(temps.at(2));

        Triangle<vertex_base_type> t{a, b, c};
        return t;
    }


    vertex_base_type get_centroid(const Face_index face_index_para) {
        const auto temps = get_all_edge_of_face(get_face_incident_edge(face_index_para));
        Vertex total(0, 0);
        for (auto temp: temps) {
            auto a = get_vertex(temp);
            total  = total + a;
        }
        auto centroid = total / temps.size();
        return centroid;
    }


    point_in_triangle_type get_vertex_in_which_face_for_test(Face_index &result_face_index,
                                                             Half_edge_index &edge_index,
                                                             vertex_base_type vertex_in) {
        result_face_index = 0;
        for (auto face: faces) {
            auto temps = if_vertex_in_face(vertex_in,
                                           face.bounding_half_edge, edge_index);
            if (temps == point_in_triangle_type::in_triangle || temps == point_in_triangle_type::on_edge)
                return temps;
            result_face_index = result_face_index + 1;
        }
        return point_in_triangle_type::out_triangle;
    }

    point_in_triangle_type if_vertex_in_face(vertex_base_type vertex_in, Half_edge_index half_edge_indices,
                                             Half_edge_index &return_half_edge_indices) {
        auto temps = get_all_edge_of_face(half_edge_indices);
        for (auto temp: temps) {
            if (get_vertex_in_the_edge_left(vertex_in, temp) == Point_2::anticlockwise::clockwise) {
                return point_in_triangle_type::out_triangle;
            }
            if (get_vertex_in_the_edge_left(vertex_in, temp) == Point_2::anticlockwise::collinear) {
                auto a = get_vertex(half_edge_indices);
                auto b = get_vertex(get_opposite_edge_index(half_edge_indices));
                if (intersect(AABB_min_max<Point_2>{a, b}, vertex_in)) {
                    return_half_edge_indices = half_edge_indices;
                    return point_in_triangle_type::on_edge;
                } else return point_in_triangle_type::out_triangle;
            }
        }
        return point_in_triangle_type::in_triangle;
    }

    Point_2::anticlockwise
    get_vertex_in_the_edge_left(vertex_base_type vertex_in, Half_edge_index half_edge_indices) {
        auto a = get_vertex(half_edge_indices);
        auto b = get_vertex(get_opposite_edge_index(half_edge_indices));
        return Point_2::is_anticlockwise(a, b, vertex_in);
    }

    AABB_min_max<vertex_base_type> calculate_aabb() {
        AABB_min_max<vertex_base_type> box;
        for (auto vertex_point: vertices) {
            box.min_point_ = vertex_base_type::min_two_point(box.min_point_, vertex_point);
            box.max_point_ = vertex_base_type::max_two_point(box.max_point_, vertex_point);
        }
        return box;
    }


    /**
     * 最后这个函数忘记是做什么的了
     * @param vertices_indices
     * @param without_hole
     * @return
     */
    bool print_all_face_vertices_indices(std::vector<Vertex_index> &vertices_indices, bool without_hole) {
        for (auto face: faces) {
            if (!without_hole || face.boundary_type != Face::BOUNDARY_TYPE::hole_face) {
                auto temp = get_all_edge_of_face(face.bounding_half_edge);
                if (temp.size() == 3) {
                    vertices_indices.push_back(temp.at(0));
                    vertices_indices.push_back(temp.at(1));
                    vertices_indices.push_back(temp.at(2));
                } else {
                    return false;
                }
            }
        }
        return true;
    }
};

#include "Half_edges_function.h"

#endif //HALF_EDGE_H
