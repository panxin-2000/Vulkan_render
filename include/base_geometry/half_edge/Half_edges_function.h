//
// Created by 潘鑫 on 2026/6/5.
//
#include "Half_edges.h"


template<typename Vertex>
bool Half_edges<Vertex>::delete_edge(const Half_edge_index current_edge) {
    const auto opposite_edge_index                                       = get_opposite_edge_index(current_edge);
    get_edge(get_edge(current_edge).pre_half_edge).next_half_edge        = get_edge(opposite_edge_index).next_half_edge;
    get_edge(get_edge(opposite_edge_index).next_half_edge).pre_half_edge = get_edge(current_edge).pre_half_edge;
    get_edge(get_edge(opposite_edge_index).pre_half_edge).next_half_edge = get_edge(current_edge).next_half_edge;
    get_edge(get_edge(current_edge).next_half_edge).pre_half_edge        = get_edge(opposite_edge_index).pre_half_edge;
    auto opposite_face                                                    = get_edge(opposite_edge_index).incident_face;
    auto current_face                                                     = get_edge(current_edge).incident_face;
    if (opposite_face != current_face) {
        faces_delete.push_back(opposite_face);
        set_face_for_edge_loop(get_edge(current_edge).pre_half_edge, current_face);
    } // 相同的话，其实是删除的同一个face,那么就不需要做任何其他的操作
    // 假设删除完成了，那么这两个边应该做什么操作呢？
    get_edge(current_edge).pre_half_edge         = opposite_edge_index;
    get_edge(current_edge).next_half_edge        = opposite_edge_index;
    get_edge(opposite_edge_index).pre_half_edge  = current_edge;
    get_edge(opposite_edge_index).next_half_edge = current_edge;
    get_edge(current_edge).incident_face         = opposite_face;
    get_edge(opposite_edge_index).incident_face  = opposite_face;
    // 将删除的两条边搞成了循环的边
    return true;
}

template<typename Vertex>
Half_edge_index Half_edges<Vertex>::insert_edge(const Half_edge_index edge_AB, Vertex point_D) {
    const auto half_edges_size    = edges.size();
    const auto vertices_size      = vertices.size();
    const auto insert_face        = edges.at(edge_AB).incident_face;
    const auto opposite_face      = edges.at(get_opposite_edge_index(edge_AB)).incident_face;
    const auto middle_point_index = vertices_size;

    //        current_edge
    // E-----A--------------B------C
    // E-----A--------------D------B------C

    const auto ba_edge = get_opposite_edge_index(edge_AB);
    const auto ab_edge = edge_AB;
    const auto bc_edge = get_next_edge_index(ab_edge);
    const auto ed_edge = get_pre_edge_index(ba_edge);
    const auto db_edge = half_edges_size;
    const auto bd_edge = half_edges_size + 1;

    // const auto vertex_index_start_point = half_edges.at(ba_edge).vertex_index;
    const auto vertex_index_end_point = edges.at(bc_edge).vertex_index;

    // 这里需要做一个判断
    if (get_next_edge_index(get_opposite_edge_index(edge_AB)) == edge_AB) {
        add_half_edge(
                      middle_point_index, bd_edge,
                      bd_edge, ab_edge, insert_face
                     );
        add_half_edge(
                      vertex_index_end_point, db_edge,
                      ba_edge, db_edge, opposite_face
                     );
        edges.at(ab_edge).next_half_edge = db_edge;
        edges.at(ba_edge).pre_half_edge  = bd_edge;
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
        edges.at(bc_edge).pre_half_edge  = db_edge;
        edges.at(ed_edge).next_half_edge = bd_edge;
        edges.at(ab_edge).next_half_edge = db_edge;
        edges.at(ba_edge).pre_half_edge  = bd_edge;
        edges.at(ba_edge).vertex_index   = middle_point_index;
        edges.at(bd_edge).vertex_index   = vertex_index_end_point;
    }
    point_D.incident_half_edge = db_edge;
    vertices.push_back(point_D);
    return db_edge;
}

template<typename Vertex>
bool Half_edges<Vertex>::flip_edge(Half_edge_index bc_or_cb_edge_index) {
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

        get_edge(x_index).pre_half_edge  = y_index;
        get_edge(x_index).next_half_edge = S_index;
        get_edge(y_index).pre_half_edge  = S_index;
        get_edge(y_index).next_half_edge = x_index;
        get_edge(S_index).pre_half_edge  = x_index;
        get_edge(S_index).next_half_edge = y_index;

        get_edge(z_index).pre_half_edge  = T_index;
        get_edge(z_index).next_half_edge = R_index;
        get_edge(R_index).pre_half_edge  = z_index;
        get_edge(R_index).next_half_edge = T_index;
        get_edge(T_index).pre_half_edge  = R_index;
        get_edge(T_index).next_half_edge = z_index;

        // 还需要更改面和点
        get_face_with_one_edge(y_index).bounding_half_edge = y_index; // 重新确认一遍 面对应的边的索引
        get_face_with_one_edge(R_index).bounding_half_edge = R_index;

        get_vertex(S_index).incident_half_edge = S_index; // 重新确认一遍顶点的入射
        get_vertex(z_index).incident_half_edge = z_index;
        set_face_for_edge_loop(y_index, get_edge(y_index).incident_face);
        set_face_for_edge_loop(R_index, get_edge(R_index).incident_face);
    }
    return true;
}

template<typename Vertex>
std::vector<Face_index> Half_edges<Vertex>::face_add_inner_point(Face_index face_index_para, Vertex add_point,
                                                                 Vertex_index &vertex_index) {
    std::vector<Face_index> new_faces;
    get_face(face_index_para).bounding_half_edge;
    const auto all_edges_of_first_face = get_all_edge_of_face(get_face(face_index_para).bounding_half_edge);
    if (all_edges_of_first_face.size() == 3) {
        const auto BC_edge           = all_edges_of_first_face.at(2);
        const auto CA_edge           = all_edges_of_first_face.at(0);
        const auto AB_edge           = all_edges_of_first_face.at(1);
        const auto half_edges_size   = edges.size();
        const auto vertices_size     = vertices.size();
        const auto faces_size        = faces.size();
        add_point.incident_half_edge = half_edges_size;
        vertex_index                 = vertices_size;
        vertices.push_back(add_point);
        auto AD_edge                     = half_edges_size + 0;
        auto DA_edge                     = half_edges_size + 1;
        auto CD_edge                     = half_edges_size + 2;
        auto DC_edge                     = half_edges_size + 3;
        auto BD_edge                     = half_edges_size + 4;
        auto DB_edge                     = half_edges_size + 5;
        const auto face_ABD              = get_edge(AB_edge).incident_face;
        const auto face_BCD              = faces_size;
        const auto face_DCA              = faces_size + 1;
        AD_edge                          = add_half_edge(get_vertices_index(AB_edge), DC_edge, CA_edge, face_DCA);
        DA_edge                          = add_half_edge(vertices_size, AB_edge, BD_edge, face_ABD);
        CD_edge                          = add_half_edge(get_vertices_index(CA_edge), DB_edge, BC_edge, face_BCD);
        DC_edge                          = add_half_edge(vertices_size, CA_edge, AD_edge, face_DCA);
        BD_edge                          = add_half_edge(get_vertices_index(BC_edge), DA_edge, AB_edge, face_ABD);
        DB_edge                          = add_half_edge(vertices_size, BC_edge, CD_edge, face_BCD);
        get_edge(AB_edge).next_half_edge = BD_edge;
        get_edge(AB_edge).pre_half_edge  = DA_edge;
        get_edge(BC_edge).next_half_edge = CD_edge;
        get_edge(BC_edge).pre_half_edge  = DB_edge;
        get_edge(CA_edge).next_half_edge = AD_edge;
        get_edge(CA_edge).pre_half_edge  = DC_edge;


        set_face_for_edge_loop(DB_edge, face_BCD);
        faces.push_back(Face{DB_edge, Face::BOUNDARY_TYPE::bounding_face});
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

template<typename Vertex>

Half_edge_index Half_edges<Vertex>::split_face(Face_index split_face, Vertex first_point,
                                               Vertex second_point) {
    auto all_edges_first_point  = get_all_edge_of_vertex(first_point);
    auto all_edges_second_point = get_all_edge_of_vertex(second_point);
    Half_edge_index first_edge;
    Half_edge_index second_edge;
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

template<typename Vertex>

Half_edge_index Half_edges<Vertex>::split_face(Half_edge_index first_edge,
                                               Vertex second_point) {
    Face_index split_face       = get_edge(first_edge).incident_face;
    auto all_edges_second_point = get_all_edge_of_vertex(second_point);
    Half_edge_index second_edge;

    for (auto edge_second_point: all_edges_second_point) {
        if (get_edge(edge_second_point).incident_face == split_face) {
            second_edge = edge_second_point;
            break;
        }
    }
    return split_face(first_edge, second_point);
}

template<typename Vertex>

Half_edge_index Half_edges<Vertex>::split_face(Half_edge_index first_edge,
                                               Half_edge_index second_edge) {
    // first_edge 是我画的图中的 E_a
    // first_edge 是我画的图中的 E_h
    const auto E_a            = first_edge;
    const auto E_h            = second_edge;
    const auto P_a            = edges.at(E_a).vertex_index;
    const auto P_b            = edges.at(E_h).vertex_index;
    const auto half_edges_size = edges.size();
    const auto vertices_size   = vertices.size();
    const auto faces_size      = faces.size();

    const auto E_f        = half_edges_size;
    const auto E_g        = half_edges_size + 1;
    const auto E_I        = get_pre_edge_index(E_a);
    const auto E_c        = get_pre_edge_index(E_h);
    const auto split_face = edges.at(E_a).incident_face;
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
    half_edges_change(E_h, pre_half_edge)  = E_f;
    half_edges_change(E_a, pre_half_edge)  = E_g;

    // 下一步需要做什么呢？将整个E_a的循环全部都设置为
    set_face_for_edge_loop(E_a, faces_size); // 将被劈开的另一边全部设置为同一个face
    faces.push_back({E_a, Face::BOUNDARY_TYPE::bounding_face});

    faces.at(split_face).bounding_half_edge = E_f; // 更新原本的face的索引

    return E_g;
}

template<typename Vertex>

Half_edge_index Half_edges<Vertex>::add_edge(const Half_edge_index current_edge, Vertex middle_point) {
    const auto half_edges_size    = edges.size();
    const auto vertices_size      = vertices.size();
    const auto faces_size         = faces.size();
    const auto middle_point_index = vertices_size;

    // 单纯的加点很好加

    const auto black_a                  = get_opposite_edge_index(current_edge);
    const auto red_b                    = current_edge;
    const auto red_c                    = half_edges_size;
    const auto blue_d                   = half_edges_size + 1;
    const auto red_e                    = half_edges_size + 2;
    const auto blue_f                   = half_edges_size + 3;
    const auto vertex_index_start_point = edges.at(black_a).vertex_index;
    const auto vertex_index_end_point   = edges.at(red_b).vertex_index;

    // const auto insert_face       = faces_size;
    const auto current_edge_face = edges.at((current_edge)).incident_face;
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
    edges.at(get_next_edge_index(red_b)).incident_face = current_edge_face;
    edges.at(get_next_edge_index(red_b)).pre_half_edge = blue_d;
    edges.at(get_pre_edge_index(red_b)).next_half_edge = blue_f;
    edges.at(red_b).next_half_edge                     = red_c;
    edges.at(red_b).pre_half_edge                      = red_e;
    edges.at(red_b).incident_face                      = faces_size;
    set_face_for_edge_loop(red_c, faces_size);
    set_face_for_edge_loop(blue_d, current_edge_face);
    faces.emplace_back(red_c, Face::BOUNDARY_TYPE::bounding_face);
    middle_point.incident_half_edge = red_e;
    vertices.push_back(middle_point);
    return red_e;
}

template<typename Vertex>
Half_edge_index Half_edges<Vertex>::add_edge(Vertex start_point,
                                             Vertex end_point) {
    auto half_edges_size           = edges.size();
    auto vertices_size             = vertices.size();
    auto faces_size                = faces.size();
    start_point.incident_half_edge = half_edges_size;
    end_point.incident_half_edge   = half_edges_size + 1;
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
    faces.emplace_back(half_edges_size + 1, Face::BOUNDARY_TYPE::hole_face);
    return half_edges_size;
}
