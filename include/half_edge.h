//
// Created by 潘鑫 on 2025/10/21.
//

#ifndef HALF_EDGE_H
#define HALF_EDGE_H
#include <vector>

#include "bounding_box.h"
#include "vector_signed_area.h"

//
// Created by 潘鑫 on 2025/10/21.
//

#include "point_3.h"
#include "point_in_on_out_triangle.h"

struct vertex_xy : public point_2 {
    using point_type = point_2;

    int incident_half_edge;

    vertex_xy(float x1, float y1) {
        x = x1;
        y = y1;
    }

    vertex_xy(point_2 point) {
        x = point.x;
        y = point.y;
    }

    vertex_xy(float x1, float y1, int incident_half_edge_1) {
        x = x1;
        y = y1;
        incident_half_edge = incident_half_edge_1;
    }

    vertex_xy operator/(unsigned long number) const {
        const vertex_xy result(x / number, y / number);
        return result;
    }

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


struct vertex_xyz : public point_3 {
    int incident_half_edge;

    vertex_xyz(float x1, float y1, float z1) {
        x = x1;
        y = y1;
        z = z1;
    }

    bool operator<(const vertex_xyz &right) const {
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

    bool operator==(const vertex_xyz &right) const {
        if (x == right.x && y == right.y && incident_half_edge == right.incident_half_edge) {
            return true;
        }
        return false;
    }
};


using Half_edge_v_index = int;
using Vertices_v_index = int;
using Face_v_index = int;
// 索引还是比较啊随意的，问题是如何建立一条边？

struct Half_edge {
    Vertices_v_index vertex_index;
    Half_edge_v_index twin_half_edge;
    Half_edge_v_index next_half_edge;
    Half_edge_v_index pre_half_edge;
    Face_v_index incident_face;
};

struct Face {
    Half_edge_v_index bounding_half_edge;

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
template<typename Vertex>
struct half_edge_struct {
    std::vector<Half_edge> half_edges;
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    std::vector<Face_v_index> faces_delete;
    using vertex_base_type = typename Vertex::point_type;

    Half_edge_v_index add_edge(Vertex start_point,
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
    Half_edge_v_index add_edge(Half_edge_v_index current_edge, Vertex middle_point) {
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
        set_face_for_new_add_edge(red_c, faces_size);
        set_face_for_new_add_edge(blue_d, current_edge_face);
        faces.push_back({red_c, Face::BOUNDARY_TYPE::bounding_face});
        middle_point.incident_half_edge = red_e;
        vertices.push_back(middle_point);
        return red_e;
    }

    Half_edge_v_index add_half_edge(Vertices_v_index vertex_index,
                                    Half_edge_v_index temp_twin_half_edge,
                                    Half_edge_v_index next_half_edge,
                                    Half_edge_v_index pre_half_edge,
                                    Face_v_index incident_face) {
        add_half_edge(vertex_index, next_half_edge, pre_half_edge, incident_face);
    }

    Half_edge_v_index add_half_edge(Vertices_v_index vertex_index,
                                    Half_edge_v_index next_half_edge,
                                    Half_edge_v_index pre_half_edge,
                                    Face_v_index incident_face) {
        int half_edges_size = half_edges.size();
        Half_edge_v_index twin_half_edge;
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

    // 如果想将原本的half_edge 中，添加一个顶点，将它分为两个edge的操作
    // 在边AB中插入一个点D
    Half_edge_v_index delete_edge(Half_edge_v_index current_edge) {
        auto opposite_edge_index = get_opposite_edge_index(current_edge);
        get_edge(get_edge(current_edge).pre_half_edge).next_half_edge = get_edge(opposite_edge_index).next_half_edge;
        get_edge(get_edge(opposite_edge_index).next_half_edge).pre_half_edge = get_edge(current_edge).pre_half_edge;
        get_edge(get_edge(opposite_edge_index).pre_half_edge).next_half_edge = get_edge(current_edge).next_half_edge;
        get_edge(get_edge(current_edge).next_half_edge).pre_half_edge = get_edge(opposite_edge_index).pre_half_edge;
        int opposite_face = get_edge(opposite_edge_index).incident_face;
        int current_face = get_edge(current_edge).incident_face;
        if (opposite_face != current_face) {
            faces_delete.push_back(opposite_face);
            set_face_for_new_add_edge(get_edge(current_edge).pre_half_edge, current_face);
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

    Half_edge_v_index insert_edge(Half_edge_v_index current_edge, Vertex d_middle_point) {
        int half_edges_size = half_edges.size();
        int vertices_size = vertices.size();
        int insert_face = half_edges.at(current_edge).incident_face;
        int opposite_face = half_edges.at(get_opposite_edge_index(current_edge)).incident_face;
        int middle_point_index = vertices_size;

        // 单纯的加点很好加

        auto ba_edge_to_da = get_opposite_edge_index(current_edge);
        auto ab_edge_to_ad = current_edge;
        auto bc_edge = get_next_edge_index(ab_edge_to_ad);
        auto ed_edge = get_pre_edge_index(ba_edge_to_da);
        auto db_edge = half_edges_size;
        auto bd_edge = half_edges_size + 1;

        int vertex_index_start_point = half_edges.at(ba_edge_to_da).vertex_index;
        int vertex_index_end_point = half_edges.at(bc_edge).vertex_index;

        // 这里需要做一个判断
        if (get_next_edge_index(get_opposite_edge_index(current_edge)) == current_edge) {
            add_half_edge(
                middle_point_index, bd_edge,
                bd_edge, ab_edge_to_ad, insert_face
            );
            add_half_edge(
                vertex_index_end_point, db_edge,
                ba_edge_to_da, db_edge, opposite_face
            );
            half_edges.at(ab_edge_to_ad).next_half_edge = db_edge;
            half_edges.at(ba_edge_to_da).pre_half_edge = bd_edge;
        } else {
            // green_f
            add_half_edge(
                middle_point_index, bd_edge,
                bc_edge, ab_edge_to_ad, insert_face
            );
            // blue_e
            add_half_edge(
                vertex_index_end_point, db_edge,
                ba_edge_to_da, ed_edge, opposite_face
            );
            half_edges.at(bc_edge).pre_half_edge = db_edge;
            half_edges.at(ed_edge).next_half_edge = bd_edge;
            half_edges.at(ab_edge_to_ad).next_half_edge = db_edge;
            half_edges.at(ba_edge_to_da).pre_half_edge = bd_edge;
            half_edges.at(ba_edge_to_da).vertex_index = middle_point_index;
            half_edges.at(bd_edge).vertex_index = vertex_index_end_point;
        }
        d_middle_point.incident_half_edge = db_edge;
        vertices.push_back(d_middle_point);
        return db_edge;
    }

    // 返回AB边的索引,有一个前置要求，要求a,b,c三个点已经是逆时针了
    Half_edge_v_index add_triangle(Vertex point_a, Vertex point_b, Vertex point_c) {
        auto half_edge_index_s = create_loop(point_b, point_a);
        auto first_half_edge = half_edge_index_s;
        add_edge(half_edge_index_s, point_c);
        return half_edge_index_s;
    }


    // 两个顶点创建一个loop，然后后创建两个面，一个是内部的面，另一个是外部的面，
    // 创建loop的时候只会创建一个面。，这个退化的线之内全部都是这个面
    // 返回 start_point 指向 end_point 索引的边，
    // 假设输入是 a , b , 那么返回的是 ab 的索引
    Half_edge_v_index create_loop(Vertex start_point,
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
    Half_edge_v_index split_face(Face_v_index split_face, Vertex first_point,
                                 Vertex second_point) {
        auto all_edges_first_point = get_all_edge_of_vertex(first_point);
        auto all_edges_second_point = get_all_edge_of_vertex(second_point);
        Half_edge_v_index first_edge;
        Half_edge_v_index second_edge;
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

    Half_edge_v_index split_face(Half_edge_v_index first_edge,
                                 Vertex second_point) {
        Face_v_index split_face = get_edge(first_edge).incident_face;
        auto all_edges_second_point = get_all_edge_of_vertex(second_point);
        Half_edge_v_index second_edge;

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
    Half_edge_v_index split_face(Half_edge_v_index first_edge,
                                 Half_edge_v_index second_edge) {
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
        half_edges.at(E_c).next_half_edge = E_g;
        half_edges.at(E_I).next_half_edge = E_f;
        half_edges.at(E_h).pre_half_edge = E_f;
        half_edges.at(E_a).pre_half_edge = E_g;

        // 下一步需要做什么呢？将整个E_a的循环全部都设置为
        set_face_for_new_add_edge(E_a, faces_size); // 将被劈开的另一边全部设置为同一个face
        faces.push_back({E_a, Face::BOUNDARY_TYPE::bounding_face});

        faces.at(split_face).bounding_half_edge = E_f; // 更新原本的face的索引

        return E_g;
    }

    Half_edge_v_index
    get_edge_from_trangle_dont_have_point(Face_v_index face_index_para, Vertices_v_index vertex_index) {
        auto edge_indices = get_all_edge_of_face(get_face(face_index_para).bounding_half_edge);
        for (auto edge_index: edge_indices) {
            if (get_edge(edge_index).vertex_index != vertex_index ||
                get_edge(get_opposite_edge_index(edge_index)).vertex_index != vertex_index) {
                return edge_index;
            }
        }
    }

    /**
     * 给一个三角形的face中间添加一个点，变更为三个face
     * @param face_index_para
     * @param add_point
     * @param vertex_index
     * @return
     */
    std::vector<Face_v_index> face_add_new_point(int face_index_para, Vertex add_point, int &vertex_index) {
        std::vector<Face_v_index> new_faces;
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


            set_face_for_new_add_edge(DB_edge, face_BCD);
            faces.push_back({DB_edge, Face::BOUNDARY_TYPE::bounding_face});
            set_face_for_new_add_edge(DC_edge, face_DCA);
            faces.push_back({DC_edge, Face::BOUNDARY_TYPE::bounding_face});

            set_face_for_new_add_edge(DA_edge, face_ABD);
            get_face(face_ABD).bounding_half_edge = DA_edge;
            new_faces.push_back(face_BCD);
            new_faces.push_back(face_DCA);
            new_faces.push_back(face_ABD);
            return new_faces;
        }
        return new_faces;
    }

    bool if_convex_quadrangle(int edge_index) {
        auto opposite_edge_index = get_opposite_edge_index(edge_index);
        auto all_edges_of_first_face = get_all_edge_of_face(edge_index);
        auto all_edges_of_second_face = get_all_edge_of_face(opposite_edge_index);
        if (all_edges_of_first_face.size() == 3 && all_edges_of_second_face.size() == 3) {
            // 这时候就可以拿到四个点了
            auto point_a = get_vertex(get_pre_edge_index(edge_index));
            auto point_b = get_vertex(edge_index);
            auto point_c = get_vertex(get_pre_edge_index(opposite_edge_index));
            auto point_d = get_vertex(opposite_edge_index);

            auto bool_1 = point_2::is_anticlockwise(point_a, point_d, point_c);
            auto bool_2 = point_2::is_anticlockwise(point_d, point_c, point_b);
            auto bool_3 = point_2::is_anticlockwise(point_c, point_b, point_a);
            auto bool_4 = point_2::is_anticlockwise(point_b, point_a, point_d);

            if (bool_1 & bool_2 & bool_3 & bool_4) {
                return true;
            }
        }
        return false;
    }

    bool flip_edge(int edge_index) {
        auto opposite_edge_index = get_opposite_edge_index(edge_index);

        if (if_convex_quadrangle(edge_index)) {
            // 全部条件都满足时，就可以进行四边形对角线的翻转操作了
            get_edge(edge_index).vertex_index = get_edge(get_pre_edge_index(edge_index)).vertex_index;
            get_edge(opposite_edge_index).vertex_index =
                    get_edge(get_pre_edge_index(opposite_edge_index)).vertex_index;
            auto x_index = get_edge(edge_index).pre_half_edge;
            auto y_index = edge_index;
            auto z_index = get_edge(edge_index).next_half_edge;
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
            set_face_for_new_add_edge(y_index, get_edge(y_index).incident_face);
            set_face_for_new_add_edge(R_index, get_edge(R_index).incident_face);
        }
    }

    bool set_face_for_new_add_edge(int half_edge_index_of_face, int faces_index) {
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

    /**
     * 这个函数的目的是为了判断是否非法？
     * 什么样才算非法呢？
     * 第四个点在前三个点组成的外接圆内
     * flip之前，一个三角形非法，另一个也是非法的，可以用两个圆随意摆放得到这个结果
     * 有一个前提条件，第四个点不能在三角形的内部
     * @param hf
     * @param half_edge_index
     * @param vertex_index
     * @return
     */
    bool legalize_edge(Half_edge_v_index half_edge_index, Vertices_v_index vertex_index) {
        // 有了一个half_edge_index 能找到那个面
        // 有了 face_index ,能够找到 三个顶点
        // 还能找到反面，还能找到反面的顶点，不在 half_edge_index 上的顶点
        // 找到 face_index 的三个顶点，计算出一个圆心和半径
        // 反面的顶点 与 圆心和半径比较，判断是否在圆内
        // 在圆内就是非法
        // 非法就需要做什么呢？
        // flip 对角线
        // 然后再进行检测，还需要检测两个内容
        // 能知道 half_edge_index 和 opposite_of_half_edge_index
        // 两个face,总共有6条边，去掉 上面的两条边，再去掉与 vertex_index 连接的两条边
        // 最后的得到剩余的两条边，重新调用这个函数，最后一个参数写什么呢？还是 vertex_index ,这个参数不需要改变
        // 然后一个问题上，这个操作我放在哪里呢？ 我觉得放在另一个文件里面会稍微好一点
        // 毕竟是操作half_edge数据结构本身的内容
        // 但是也没有改变太多的内容
        auto face_index = get_face_index(half_edge_index);
        auto all_edges_of_first_face = get_all_edge_of_face(half_edge_index);
        auto all_edges_of_second_face = get_all_edge_of_face(get_opposite_edge_index(half_edge_index));
        // 下面的判断里面少了一步确定非凹，两个三角形组成了一个凹四边形 todo:
        if (all_edges_of_first_face.size() == 3 &&
            all_edges_of_second_face.size() == 3 &&
            if_convex_quadrangle(half_edge_index) &&
            get_face(get_edge(get_opposite_edge_index(half_edge_index)).incident_face).boundary_type !=
            Face::BOUNDARY_TYPE::hole_face) {
            //    B----------D
            //    *  *       *
            //    *    *     *
            //    *      *   *
            //    *        * *
            //    A----------C
            //  BC 两个点相互交换应该是没有问题的
            auto BC_edge = half_edge_index;

            auto AC_or_AB_edge = get_pre_edge_index(half_edge_index);
            auto AB_or_AC_edge = get_next_edge_index(half_edge_index);

            // 确定是 AC_or_AB 而不是 DB_or_DC
            if (get_edge(AC_or_AB_edge).vertex_index == vertex_index ||
                get_edge(AB_or_AC_edge).vertex_index == vertex_index) {
                AC_or_AB_edge = get_pre_edge_index(get_opposite_edge_index(half_edge_index));
                AB_or_AC_edge = get_next_edge_index(get_opposite_edge_index(half_edge_index));
            }
            auto A_point = get_vertex(AC_or_AB_edge);
            auto B_point = get_vertex(half_edge_index);
            auto D_point = vertices.at(vertex_index);
            auto C_point = get_vertex(get_opposite_edge_index(half_edge_index));
            //

            auto centre = point_2::centre_of_a_circle(A_point, B_point, C_point);
            if (point_2::distance_compare(D_point - centre, C_point - centre)) {
                // 当前是合法的
            } else {
                // 当前是非法的，需要执行flip操作
                flip_edge(half_edge_index);
                // half_edge_index 这个索引并没有改变
                // 但是边需要变更了，需要变更为 AB 或者 AC ，但是不能是 BD 或者 CD ,前面添加条件确定了
                legalize_edge(AC_or_AB_edge, vertex_index);
                legalize_edge(AB_or_AC_edge, vertex_index);
            }
        }
        return false;
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

    Vertex &get_vertex(int incident_half_edge) {
        // int same_edge_index = get_same_edge_index(incident_half_edge);
        // 当时为什么会加这么一行，想起来也是有点离谱的
        int vertex_index_end_point = half_edges.at(incident_half_edge).vertex_index;
        return vertices.at(vertex_index_end_point);
    }

    Half_edge_v_index &get_edge_incident_edge(int face_index) {
        return faces.at(face_index).bounding_half_edge;
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

    [[nodiscard]] Half_edge_v_index get_next_edge_index(const int incident_half_edge) const {
        const int result = half_edges.at(incident_half_edge).next_half_edge;
        return result;
    }

    [[nodiscard]] Half_edge_v_index get_pre_edge_index(const int incident_half_edge) const {
        const int result = half_edges.at(incident_half_edge).pre_half_edge;
        return result;
    }

    [[nodiscard]] Half_edge_v_index get_twin_edge_index(const int incident_half_edge) const {
        const int result = half_edges.at(incident_half_edge).twin_half_edge;
        return result;
    }

    [[nodiscard]] Half_edge_v_index get_opposite_edge_index(const int incident_half_edge) const {
        const int result = half_edges.at(incident_half_edge).twin_half_edge;
        return result;
    }

    [[nodiscard]] Vertices_v_index get_vertices_index(const int incident_half_edge) const {
        const int result = half_edges.at(incident_half_edge).vertex_index;
        return result;
    }

    int get_face_index(Half_edge_v_index edge_index) {
        return half_edges.at(edge_index).incident_face;
    }

    std::vector<Half_edge_v_index> get_all_edge_of_vertex(int vertex_index) {
        std::vector<Half_edge_v_index> result;
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

    std::vector<Face_v_index> get_all_face_of_vertex(int vertex_index) {
        auto all_edges = get_all_edge_of_vertex(vertex_index);
        std::vector<Face_v_index> result;
        for (Half_edge_v_index single_edge: all_edges) {
            result.push_back(get_edge(single_edge).incident_face);
        }
        return result;
    }


    std::vector<Half_edge_v_index> get_all_edge_of_face(int half_edge_index_of_face) {
        std::vector<Half_edge_v_index> result;
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

    std::vector<Half_edge_v_index> get_all_edge_of_face(Face face) {
        return get_all_edge_of_face(face.bounding_half_edge);
    }

    // point_2 get_vertex(half_edge_index half_edge_indices) const {
    //     point_2 temp_point{};
    //     temp_point.x = vertices.at(half_edges.at(half_edge_indices).vertex_index).x;
    //     temp_point.y = vertices.at(half_edges.at(half_edge_indices).vertex_index).y;
    //     return temp_point;
    // }

    std::vector<point_2> get_vertices(const std::vector<Half_edge_v_index> &half_edge_indices) const {
        std::vector<point_2> segments{};
        for (auto half_edge_index: half_edge_indices) {
            point_2 temp_point{};
            temp_point.x = vertices.at(half_edges.at(half_edge_index).vertex_index).x;
            temp_point.y = vertices.at(half_edges.at(half_edge_index).vertex_index).y;
            segments.push_back(temp_point);
        }
        return segments;
    }

    /**
     *
     * @tparam T
     * @param result_segments
     * @param without_hole true 时 不输出洞， false 输出洞
     * @return
     */
    template<typename T>
    bool print_all_face_vertices(T &result_segments, bool without_hole) {
        for (auto face: faces) {
            if (!without_hole || face.boundary_type != Face::BOUNDARY_TYPE::hole_face) {
                auto temp = get_all_edge_of_face(face.bounding_half_edge);
                if (temp.size() == 3) {
                    auto a = get_vertex(temp.at(0));
                    auto b = get_vertex(temp.at(1));
                    auto c = get_vertex(temp.at(2));

                    Triangle<vertex_base_type> t{
                        static_cast<vertex_base_type>(a),
                        static_cast<vertex_base_type>(b),
                        static_cast<vertex_base_type>(c)
                    };
                    result_segments.push_back(t);
                } else {
                    return false;
                }
            }
        }
        return true;
    }

    vertex_base_type get_centroid(int face_index_para) {
        auto temps = get_all_edge_of_face(get_edge_incident_edge(face_index_para));
        Vertex total(0, 0);
        for (auto temp: temps) {
            auto a = get_vertex(temp);
            total = total + a;
        }
        auto centroid = total / temps.size();
        return centroid;
    }


    point_in_triangle_type get_vertex_in_witch_face_test(Face_v_index &result_face_index, Half_edge_v_index &edge_index,
                                                         vertex_base_type vertex_in) {
        result_face_index = 0;
        for (auto face: faces) {
            auto temps = get_vertex_in_face(vertex_in,
                                            face.bounding_half_edge, edge_index);
            if (temps == point_in_triangle_type::in_triangle || temps == point_in_triangle_type::on_edge)
                return temps;
            result_face_index = result_face_index + 1;
        }
    }

    point_in_triangle_type get_vertex_in_face(vertex_base_type vertex_in, Half_edge_v_index half_edge_indices,
                                              Half_edge_v_index &return_half_edge_indices) {
        auto temps = get_all_edge_of_face(half_edge_indices);
        for (auto temp: temps) {
            if (get_vertex_in_the_edge_left(vertex_in, temp) == point_2::anticlockwise::clockwise) {
                return point_in_triangle_type::out_triangle;
            }
            if (get_vertex_in_the_edge_left(vertex_in, temp) == point_2::anticlockwise::collinear) {
                auto a = get_vertex(half_edge_indices);
                auto b = get_vertex(get_opposite_edge_index(half_edge_indices));
                if (on_segment_bounding_box(a, b, vertex_in)) {
                    return_half_edge_indices = half_edge_indices;
                    return point_in_triangle_type::on_edge;
                } else return point_in_triangle_type::out_triangle;
            }
        }
        return point_in_triangle_type::in_triangle;
    }

    point_2::anticlockwise
    get_vertex_in_the_edge_left(vertex_base_type vertex_in, Half_edge_v_index half_edge_indices) {
        auto a = get_vertex(half_edge_indices);
        auto b = get_vertex(get_opposite_edge_index(half_edge_indices));
        return point_2::is_anticlockwise(a, b, vertex_in);
    }

    AABB<vertex_base_type> calculate_aabb() {
        AABB<vertex_base_type> box;
        for (auto vertex_point: vertices) {
            box.min_point = vertex_base_type::min_two_point(box.min_point, vertex_point);
            box.max_point = vertex_base_type::max_two_point(box.max_point, vertex_point);
        }
        return box;
    }


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
