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




using half_edge_index = int;
using vertex_index = int;
using face_index = int;
// 索引还是比较啊随意的，问题是如何建立一条边？
#include "base_geometry/half_edge/half_edge.h"
#include "base_geometry/half_edge/face.h"


// 还可以把half_edge和face也做出模版参数,方便进行扩展
template<typename Vertex>
struct half_edge_struct {
    std::vector<Half_edge> half_edges;
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    std::vector<face_index> faces_delete;
    using vertex_base_type = typename Vertex::point_type;

    struct History {
        std::vector<std::pair<vertex_index, Vertex> > vertices;
        std::vector<std::pair<half_edge_index, Half_edge> > half_edges;
        std::vector<std::pair<face_index, Face> > faces;
    };

    std::array<History, 10> history;
    uint16_t current_history = 0;

    // 还需要添加检查，不要造成重复添加值
#define half_edges_change(index,detail)                            \
    if constexpr(false){                                           \
    auto &temp = history[current_history].half_edges;              \
    temp.push_back(std::make_pair(index, half_edges.at(index)));   \
    }                                                              \
    half_edges.at(index).detail

    half_edge_index add_edge(Vertex start_point,
                             Vertex end_point) {
        int half_edges_size = half_edges.size();
        int vertices_size = vertices.size();
        start_point.incident_half_edge = half_edges_size;
        end_point.incident_half_edge = half_edges_size + 1;
        vertices.push_back(start_point);
        vertices.push_back(end_point);
        add_half_edge(vertices_size, half_edges_size + 1, 0, 0);
        // 上面这条边插入的是什么呢？它的vertex 和 边是一一对应的，知道一条边，能够知道它的起点
        add_half_edge(vertices_size + 1, half_edges_size, 0, 0);
        return half_edges_size;
    }

    // 之后再在内部的面上增加顶点，然后就有一个问题，
    // 如果之前内部的面是空的话，那么不需要创建新的face
    // 如果不为空的话，那么是什么样子的呢？
    // 需要新建立一个面，并更改掉一些原本的内容
    half_edge_index add_edge(half_edge_index current_edge, Vertex middle_point) {
        int half_edges_size = half_edges.size();
        int vertices_size = vertices.size();
        int faces_size = faces.size();
        auto middle_point_index = vertices_size;

        // 单纯的加点很好加

        auto black_a = get_opposite_edge_index(current_edge);
        auto red_b = current_edge;
        auto red_c = half_edges_size;
        auto blue_d = half_edges_size + 1;
        auto red_e = half_edges_size + 2;
        auto blue_f = half_edges_size + 3;
        auto vertex_index_start_point = half_edges.at(black_a).vertex_index;
        auto vertex_index_end_point = half_edges.at(red_b).vertex_index;

        auto insert_face = faces_size;
        auto current_edge_face = half_edges.at((current_edge)).incident_face;
        // 这里引发的问题，但是应该改过之后的为正确的

        add_half_edge(
            vertex_index_start_point, blue_d,
            red_e, red_b, faces_size
        );
        add_half_edge(
            middle_point_index, half_edges_size,
            get_next_edge_index(red_b), blue_f, current_edge_face
        );
        add_half_edge(
            middle_point_index, blue_f,
            red_b, red_c, faces_size
        );
        add_half_edge(
            vertex_index_end_point, red_e,
            blue_d, get_pre_edge_index(red_b), current_edge_face
        );
        // 添加新的四条边也是能够添加的
        // 问题是原本的边应该怎么动？
        // 判断一下get_next(get_opposite(current_edge)) == current_edge
        // 不需要，直接执行就好，能够直接满足两种条件
        half_edges.at(get_next_edge_index(red_b)).incident_face = current_edge_face;
        half_edges.at(get_next_edge_index(red_b)).pre_half_edge = blue_d;
        half_edges.at(get_pre_edge_index(red_b)).next_half_edge = blue_f;
        half_edges.at(red_b).next_half_edge = red_c;
        half_edges.at(red_b).pre_half_edge = red_e;
        half_edges.at(red_b).incident_face = faces_size;
        set_face_for_edge_loop(red_c, faces_size);
        set_face_for_edge_loop(blue_d, current_edge_face);
        faces.push_back({red_c, Face::BOUNDARY_TYPE::bounding_face});
        middle_point.incident_half_edge = red_e;
        vertices.push_back(middle_point);
        return red_e;
    }

    half_edge_index add_half_edge(vertex_index vertex_index,
                                  half_edge_index temp_twin_half_edge,
                                  half_edge_index next_half_edge,
                                  half_edge_index pre_half_edge,
                                  face_index incident_face) {
        add_half_edge(vertex_index, next_half_edge, pre_half_edge, incident_face);
    }

    half_edge_index add_half_edge(vertex_index vertex_index,
                                  half_edge_index next_half_edge,
                                  half_edge_index pre_half_edge,
                                  face_index incident_face) {
        int half_edges_size = half_edges.size();
        half_edge_index twin_half_edge;
        // half_edges_size 为 0 时， twin_half_edge = 1
        // half_edges_size 为 1 时， twin_half_edge = 0
        if (half_edges_size % 2 == 0) {
            twin_half_edge = half_edges_size + 1;
        } else {
            twin_half_edge = half_edges_size - 1;
        }
        Half_edge temp;
        temp.next_half_edge = next_half_edge;
        temp.pre_half_edge = pre_half_edge;
        temp.incident_face = incident_face;
        temp.twin_half_edge = twin_half_edge;
        temp.vertex_index = vertex_index;


        half_edges.push_back(temp);
        return half_edges_size;
    }

    half_edge_index delete_edge(half_edge_index current_edge) {
        auto opposite_edge_index = get_opposite_edge_index(current_edge);
        get_edge(get_edge(current_edge).pre_half_edge).next_half_edge = get_edge(opposite_edge_index).next_half_edge;
        get_edge(get_edge(opposite_edge_index).next_half_edge).pre_half_edge = get_edge(current_edge).pre_half_edge;
        get_edge(get_edge(opposite_edge_index).pre_half_edge).next_half_edge = get_edge(current_edge).next_half_edge;
        get_edge(get_edge(current_edge).next_half_edge).pre_half_edge = get_edge(opposite_edge_index).pre_half_edge;
        int opposite_face = get_edge(opposite_edge_index).incident_face;
        int current_face = get_edge(current_edge).incident_face;
        if (opposite_face != current_face) {
            faces_delete.push_back(opposite_face);
            set_face_for_edge_loop(get_edge(current_edge).pre_half_edge, current_face);
        } // 相同的话，其实是删除的同一个face,那么就不需要做任何其他的操作
        // 假设删除完成了，那么这两个边应该做什么操作呢？
        get_edge(current_edge).pre_half_edge = opposite_edge_index;
        get_edge(current_edge).next_half_edge = opposite_edge_index;
        get_edge(opposite_edge_index).pre_half_edge = current_edge;
        get_edge(opposite_edge_index).next_half_edge = current_edge;
        get_edge(current_edge).incident_face = opposite_face;
        get_edge(opposite_edge_index).incident_face = opposite_face;
        // 将删除的两条边搞成了循环的边
    }

    /**
     * 在AB边上添加一个点，现在这个边变成了ADB，返回DB这条边
     * @param current_edge
     * @param d_point
     * @return
     */
    half_edge_index insert_edge(half_edge_index current_edge, Vertex d_point) {
        int half_edges_size = half_edges.size();
        int vertices_size = vertices.size();
        int insert_face = half_edges.at(current_edge).incident_face;
        int opposite_face = half_edges.at(get_opposite_edge_index(current_edge)).incident_face;
        int middle_point_index = vertices_size;

        //        current_edge
        // E-----A--------------B------C
        // E-----A--------------D------B------C

        auto ba_edge = get_opposite_edge_index(current_edge);
        auto ab_edge = current_edge;
        auto bc_edge = get_next_edge_index(ab_edge);
        auto ed_edge = get_pre_edge_index(ba_edge);
        auto db_edge = half_edges_size;
        auto bd_edge = half_edges_size + 1;

        int vertex_index_start_point = half_edges.at(ba_edge).vertex_index;
        int vertex_index_end_point = half_edges.at(bc_edge).vertex_index;

        // 这里需要做一个判断
        if (get_next_edge_index(get_opposite_edge_index(current_edge)) == current_edge) {
            add_half_edge(
                middle_point_index, bd_edge,
                bd_edge, ab_edge, insert_face
            );
            add_half_edge(
                vertex_index_end_point, db_edge,
                ba_edge, db_edge, opposite_face
            );
            half_edges.at(ab_edge).next_half_edge = db_edge;
            half_edges.at(ba_edge).pre_half_edge = bd_edge;
        } else {
            // green_f
            add_half_edge(
                middle_point_index, bd_edge,
                bc_edge, ab_edge, insert_face
            );
            // blue_e
            add_half_edge(
                vertex_index_end_point, db_edge,
                ba_edge, ed_edge, opposite_face
            );
            half_edges.at(bc_edge).pre_half_edge = db_edge;
            half_edges.at(ed_edge).next_half_edge = bd_edge;
            half_edges.at(ab_edge).next_half_edge = db_edge;
            half_edges.at(ba_edge).pre_half_edge = bd_edge;
            half_edges.at(ba_edge).vertex_index = middle_point_index;
            half_edges.at(bd_edge).vertex_index = vertex_index_end_point;
        }
        d_point.incident_half_edge = db_edge;
        vertices.push_back(d_point);
        return db_edge;
    }

    // 返回AB边的索引,有一个前置要求，要求a,b,c三个点已经是逆时针了
    half_edge_index add_triangle(Vertex point_a, Vertex point_b, Vertex point_c) {
        auto half_edge_index_s = create_loop(point_b, point_a);
        auto first_half_edge = half_edge_index_s;
        add_edge(half_edge_index_s, point_c);
        return half_edge_index_s;
    }


    // 两个顶点创建一个loop，然后后创建两个面，一个是内部的面，另一个是外部的面，
    // 创建loop的时候只会创建一个面。，这个退化的线之内全部都是这个面
    // 返回 start_point 指向 end_point 索引的边，
    // 假设输入是 a , b , 那么返回的是 ab 的索引
    half_edge_index create_loop(Vertex start_point,
                                Vertex end_point) {
        auto half_edges_size = half_edges.size();
        auto vertices_size = vertices.size();
        auto faces_size = faces.size();
        start_point.incident_half_edge = half_edges_size;
        end_point.incident_half_edge = half_edges_size + 1;
        vertices.push_back(start_point);
        vertices.push_back(end_point);
        add_half_edge(
            vertices_size, half_edges_size + 1,
            half_edges_size + 1, half_edges_size + 1, faces_size
        );
        add_half_edge(
            vertices_size + 1, half_edges_size,
            half_edges_size, half_edges_size, faces_size
        );
        Face temp_face;
        temp_face.boundary_type = Face::BOUNDARY_TYPE::hole_face;
        temp_face.bounding_half_edge = half_edges_size + 1;
        faces.push_back(temp_face);
        return half_edges_size;
    }

    // 在一个两个顶点直接插入一条边，将原本的一个face，分为两个face
    half_edge_index split_face(face_index split_face, Vertex first_point,
                               Vertex second_point) {
        auto all_edges_first_point = get_all_edge_of_vertex(first_point);
        auto all_edges_second_point = get_all_edge_of_vertex(second_point);
        half_edge_index first_edge;
        half_edge_index second_edge;
        for (auto edge_first_point: all_edges_first_point) {
            if (get_edge(edge_first_point).incident_face == split_face) {
                first_edge = edge_first_point;
                break;
            }
        }
        for (auto edge_second_point: all_edges_second_point) {
            if (get_edge(edge_second_point).incident_face == split_face) {
                second_edge = edge_second_point;
                break;
            }
        }
        return split_face(first_edge, second_point);
    }

    half_edge_index split_face(half_edge_index first_edge,
                               Vertex second_point) {
        face_index split_face = get_edge(first_edge).incident_face;
        auto all_edges_second_point = get_all_edge_of_vertex(second_point);
        half_edge_index second_edge;

        for (auto edge_second_point: all_edges_second_point) {
            if (get_edge(edge_second_point).incident_face == split_face) {
                second_edge = edge_second_point;
                break;
            }
        }
        return split_face(first_edge, second_point);
    }

    // 最后开始做的时候没有写注释
    // 参考上面的代码，才能知道具体的内容是什么
    half_edge_index split_face(half_edge_index first_edge,
                               half_edge_index second_edge) {
        // first_edge 是我画的图中的 E_a
        // first_edge 是我画的图中的 E_h
        auto E_a = first_edge;
        auto E_h = second_edge;
        auto P_a = half_edges.at(E_a).vertex_index;
        auto P_b = half_edges.at(E_h).vertex_index;
        int half_edges_size = half_edges.size();
        int vertices_size = vertices.size();
        int faces_size = faces.size();

        auto E_f = half_edges_size;
        auto E_g = half_edges_size + 1;
        auto E_I = get_pre_edge_index(E_a);
        auto E_c = get_pre_edge_index(E_h);
        auto split_face = half_edges.at(E_a).incident_face;
        // 拿到我要劈开的环的索引
        // E_f
        add_half_edge(
            P_a, E_g,
            E_h, E_I, split_face
        );
        // E_g
        add_half_edge(
            P_b, E_f,
            E_a, E_c, faces_size
        );


        half_edges_change(E_c, next_half_edge) = E_g;
        half_edges_change(E_I, next_half_edge) = E_f;
        half_edges_change(E_h, pre_half_edge) = E_f;
        half_edges_change(E_a, pre_half_edge) = E_g;

        // 下一步需要做什么呢？将整个E_a的循环全部都设置为
        set_face_for_edge_loop(E_a, faces_size); // 将被劈开的另一边全部设置为同一个face
        faces.push_back({E_a, Face::BOUNDARY_TYPE::bounding_face});

        faces.at(split_face).bounding_half_edge = E_f; // 更新原本的face的索引

        return E_g;
    }

    half_edge_index
    get_ab_edge_from_face_abc(face_index face_index_para, vertex_index vertex_c_index) {
        auto edge_indices = get_all_edge_of_face(get_face(face_index_para).bounding_half_edge);
        for (auto edge_index: edge_indices) {
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
    std::vector<face_index> face_add_new_point(int face_index_para, Vertex add_point, int &vertex_index) {
        std::vector<face_index> new_faces;
        get_face(face_index_para).bounding_half_edge;
        auto all_edges_of_first_face = get_all_edge_of_face(get_face(face_index_para).bounding_half_edge);
        if (all_edges_of_first_face.size() == 3) {
            auto BC_edge = all_edges_of_first_face.at(2);
            auto CA_edge = all_edges_of_first_face.at(0);
            auto AB_edge = all_edges_of_first_face.at(1);
            int half_edges_size = half_edges.size();
            int vertices_size = vertices.size();
            int faces_size = faces.size();
            add_point.incident_half_edge = half_edges_size;
            vertex_index = vertices_size;
            vertices.push_back(add_point);
            auto AD_edge = half_edges_size;
            auto DA_edge = half_edges_size + 1;
            auto CD_edge = half_edges_size + 2;
            auto DC_edge = half_edges_size + 3;
            auto BD_edge = half_edges_size + 4;
            auto DB_edge = half_edges_size + 5;
            auto face_ABD = get_edge(AB_edge).incident_face;
            auto face_BCD = faces_size;
            auto face_DCA = faces_size + 1;
            AD_edge = add_half_edge(get_vertices_index(AB_edge), DC_edge, CA_edge, face_DCA);
            DA_edge = add_half_edge(vertices_size, AB_edge, BD_edge, face_ABD);
            CD_edge = add_half_edge(get_vertices_index(CA_edge), DB_edge, BC_edge, face_BCD);
            DC_edge = add_half_edge(vertices_size, CA_edge, AD_edge, face_DCA);
            BD_edge = add_half_edge(get_vertices_index(BC_edge), DA_edge, AB_edge, face_ABD);
            DB_edge = add_half_edge(vertices_size, BC_edge, CD_edge, face_BCD);
            get_edge(AB_edge).next_half_edge = BD_edge;
            get_edge(AB_edge).pre_half_edge = DA_edge;
            get_edge(BC_edge).next_half_edge = CD_edge;
            get_edge(BC_edge).pre_half_edge = DB_edge;
            get_edge(CA_edge).next_half_edge = AD_edge;
            get_edge(CA_edge).pre_half_edge = DC_edge;


            set_face_for_edge_loop(DB_edge, face_BCD);
            faces.push_back({DB_edge, Face::BOUNDARY_TYPE::bounding_face});
            set_face_for_edge_loop(DC_edge, face_DCA);
            faces.push_back({DC_edge, Face::BOUNDARY_TYPE::bounding_face});

            set_face_for_edge_loop(DA_edge, face_ABD);
            get_face(face_ABD).bounding_half_edge = DA_edge;
            new_faces.push_back(face_BCD);
            new_faces.push_back(face_DCA);
            new_faces.push_back(face_ABD);
            return new_faces;
        }
        return new_faces;
    }


    /***************************下面的代码都不会更改原本的结构***********************************/
    /**
     * 一个边会有两个相邻的面，判断这两个相邻的面组成的多边形是否是凸的
     * @param edge_index
     * @return
     */
    bool if_convex_quadrangle(int edge_index) {
        auto opposite_edge_index = get_opposite_edge_index(edge_index);
        auto all_edges_of_first_face = get_all_edge_of_face(edge_index);
        auto all_edges_of_second_face = get_all_edge_of_face(opposite_edge_index);
        if (all_edges_of_first_face.size() == 3 && all_edges_of_second_face.size() == 3) {
            // 这时候就可以拿到四个点了
            Trapezoid temp{
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
    bool flip_edge(int bc_or_cb_edge_index) {
        auto opposite_edge_index = get_opposite_edge_index(bc_or_cb_edge_index);

        if (if_convex_quadrangle(bc_or_cb_edge_index)) {
            // 全部条件都满足时，就可以进行四边形对角线的翻转操作了
            get_edge(bc_or_cb_edge_index).vertex_index = get_edge(get_pre_edge_index(bc_or_cb_edge_index)).vertex_index;
            get_edge(opposite_edge_index).vertex_index =
                    get_edge(get_pre_edge_index(opposite_edge_index)).vertex_index;
            auto x_index = get_edge(bc_or_cb_edge_index).pre_half_edge;
            auto y_index = bc_or_cb_edge_index;
            auto z_index = get_edge(bc_or_cb_edge_index).next_half_edge;
            auto R_index = opposite_edge_index;
            auto S_index = get_edge(opposite_edge_index).next_half_edge;
            auto T_index = get_edge(opposite_edge_index).pre_half_edge;

            get_edge(x_index).pre_half_edge = y_index;
            get_edge(x_index).next_half_edge = S_index;
            get_edge(y_index).pre_half_edge = S_index;
            get_edge(y_index).next_half_edge = x_index;
            get_edge(S_index).pre_half_edge = x_index;
            get_edge(S_index).next_half_edge = y_index;

            get_edge(z_index).pre_half_edge = T_index;
            get_edge(z_index).next_half_edge = R_index;
            get_edge(R_index).pre_half_edge = z_index;
            get_edge(R_index).next_half_edge = T_index;
            get_edge(T_index).pre_half_edge = R_index;
            get_edge(T_index).next_half_edge = z_index;

            // 还需要更改面和点
            get_face_with_one_edge(y_index).bounding_half_edge = y_index; // 重新确认一遍 面对应的边的索引
            get_face_with_one_edge(R_index).bounding_half_edge = R_index;

            get_vertex(S_index).incident_half_edge = S_index; // 重新确认一遍顶点的入射
            get_vertex(z_index).incident_half_edge = z_index;
            set_face_for_edge_loop(y_index, get_edge(y_index).incident_face);
            set_face_for_edge_loop(R_index, get_edge(R_index).incident_face);
        }
    }

    bool set_face_for_edge_loop(int half_edge_index_of_face, int faces_index) {
        int faces_size = faces_index;
        int current_half_edge_index = half_edge_index_of_face;
        int first_half_edge = current_half_edge_index;
        while (true) {
            half_edges.at(current_half_edge_index).incident_face = faces_size;
            auto next_half_edge = get_next_edge_index(current_half_edge_index);
            current_half_edge_index = next_half_edge;
            if (current_half_edge_index == first_half_edge) {
                return true;
            }
        }
        return false;
    }


    Segment<Point_2> get_segment(int incident_half_edge) {
        // 稍微有一点点的问题啊？
        int twin_half_edge = half_edges.at(incident_half_edge).twin_half_edge;
        int vertex_index_start_point = half_edges.at(incident_half_edge).vertex_index;
        int vertex_index_end_point = half_edges.at(twin_half_edge).vertex_index;

        Point_2 start_point{
            vertices.at(vertex_index_start_point).x,
            vertices.at(vertex_index_start_point).y
        };
        Point_2 end_point{
            vertices.at(vertex_index_end_point).x,
            vertices.at(vertex_index_end_point).y
        };
        Segment<Point_2> result{start_point, end_point};
        return result;
    }

    std::vector<Segment<> > get_all_segments() {
        std::vector<Segment<> > result;
        for (int i = 0; i < half_edges.size(); ++i, ++i) {
            auto segment = get_segment(i);
            result.emplace_back(segment);
        }
        return result;
    }

    int get_same_edge_index(const int incident_half_edge) {
        return incident_half_edge - (incident_half_edge % 2);
    }

    Vertex &get_vertex(const int incident_half_edge) {
        int vertex_index_end_point = half_edges.at(incident_half_edge).vertex_index;
        return vertices.at(vertex_index_end_point);
    }


    Half_edge &get_edge(int incident_half_edge) {
        return half_edges.at(incident_half_edge);
    }

    Face &get_face_with_one_edge(int incident_half_edge) {
        return faces.at(get_edge(incident_half_edge).incident_face);
    }

    Face &get_face(int face_index) {
        return faces.at(face_index);
    }

    [[nodiscard]] half_edge_index &get_face_incident_edge(const int face_index) {
        return faces.at(face_index).bounding_half_edge;
    }

    [[nodiscard]] half_edge_index get_next_edge_index(const int half_edge_index) const {
        const int result = half_edges.at(half_edge_index).next_half_edge;
        return result;
    }

    [[nodiscard]] half_edge_index get_pre_edge_index(const int half_edge_index) const {
        const int result = half_edges.at(half_edge_index).pre_half_edge;
        return result;
    }

    [[nodiscard]] half_edge_index get_twin_edge_index(const int half_edge_index) const {
        const int result = half_edges.at(half_edge_index).twin_half_edge;
        return result;
    }

    [[nodiscard]] half_edge_index get_opposite_edge_index(const int half_edge_index) const {
        const int result = half_edges.at(half_edge_index).twin_half_edge;
        return result;
    }

    [[nodiscard]] vertex_index get_vertices_index(const int half_edge_index) const {
        const int result = half_edges.at(half_edge_index).vertex_index;
        return result;
    }

    [[nodiscard]] face_index get_face_index(half_edge_index half_edge_index) {
        return half_edges.at(half_edge_index).incident_face;
    }

    std::vector<half_edge_index> get_all_edge_of_vertex(int vertex_index) {
        std::vector<half_edge_index> result;
        int current_half_edge_index = vertices.at(vertex_index).incident_half_edge;
        int first_half_edge = current_half_edge_index;
        while (true) {
            auto opposite_edge = get_opposite_edge_index(current_half_edge_index);
            result.push_back(opposite_edge);
            auto next_half_edge = get_next_edge_index(opposite_edge);
            current_half_edge_index = next_half_edge;
            if (current_half_edge_index == first_half_edge) {
                return result;
            }
        }
    }

    std::vector<face_index> get_all_face_of_vertex(int vertex_index) {
        auto all_edges = get_all_edge_of_vertex(vertex_index);
        std::vector<face_index> result;
        for (half_edge_index single_edge: all_edges) {
            result.push_back(get_edge(single_edge).incident_face);
        }
        return result;
    }

    std::vector<half_edge_index> get_all_edge_of_face(const int half_edge_index_of_face) {
        std::vector<half_edge_index> result;
        int current_half_edge_index = half_edge_index_of_face;
        int first_half_edge = current_half_edge_index;
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
                int_put_face.boundary_type = face.boundary_type;
                return true;
            }
        }
        return false;
    }


    [[nodiscard]] std::vector<Point_2> get_vertices(const std::vector<half_edge_index> &half_edge_indices) const {
        std::vector<Point_2> points{};
        for (auto half_edge_index: half_edge_indices) {
            Point_2 temp_point{};
            temp_point.x = vertices.at(half_edges.at(half_edge_index).vertex_index).x;
            temp_point.y = vertices.at(half_edges.at(half_edge_index).vertex_index).y;
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

    Triangle<vertex_base_type> get_triangle_face_vertex(const int face_index_para) {
        auto temps = get_all_edge_of_face(get_face_incident_edge(face_index_para));
        auto a = get_vertex(temps.at(0));
        auto b = get_vertex(temps.at(1));
        auto c = get_vertex(temps.at(2));

        Triangle<vertex_base_type> t{a, b, c};
        return t;
    }


    vertex_base_type get_centroid(const int face_index_para) {
        const auto temps = get_all_edge_of_face(get_face_incident_edge(face_index_para));
        Vertex total(0, 0);
        for (auto temp: temps) {
            auto a = get_vertex(temp);
            total = total + a;
        }
        auto centroid = total / temps.size();
        return centroid;
    }


    point_in_triangle_type get_vertex_in_which_face_for_test(face_index &result_face_index,
                                                             half_edge_index &edge_index,
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

    point_in_triangle_type if_vertex_in_face(vertex_base_type vertex_in, half_edge_index half_edge_indices,
                                             half_edge_index &return_half_edge_indices) {
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
    get_vertex_in_the_edge_left(vertex_base_type vertex_in, half_edge_index half_edge_indices) {
        auto a = get_vertex(half_edge_indices);
        auto b = get_vertex(get_opposite_edge_index(half_edge_indices));
        return Point_2::is_anticlockwise(a, b, vertex_in);
    }

    AABB_min_max<vertex_base_type> calculate_aabb() {
        AABB_min_max<vertex_base_type> box;
        for (auto vertex_point: vertices) {
            box.min_point = vertex_base_type::min_two_point(box.min_point, vertex_point);
            box.max_point = vertex_base_type::max_two_point(box.max_point, vertex_point);
        }
        return box;
    }


    /**
     * 最后这个函数忘记是做什么的了
     * @param vertices_indices
     * @param without_hole
     * @return
     */
    bool print_all_face_vertices_indices(std::vector<int> &vertices_indices, bool without_hole) {
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


#endif //HALF_EDGE_H
