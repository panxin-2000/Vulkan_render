//
// Created by 潘鑫 on 2025/11/12.
//

#ifndef TEST_DELAUNAY_TRIANGULATION_H
#define TEST_DELAUNAY_TRIANGULATION_H
#include <glm/fwd.hpp>
#include "ear_clip.h"
#include "base_element/half_edge/half_edge_struct.h"
#include "triangle_graph.h"

namespace delaunay_triangulation {
    // 并不是算事一个类，只能算是一个方法
    //                           *---------------------------------------C
    //                           *                   *                   *
    //                           *                   *                   *
    //                           *                   *                   *
    //       A-------------------D-------------------*-------------------*
    //       *                   *  * *  *  *  *  *  *
    //       *                   *  * *  *  *  *  *  *
    //       *                   *  * *  *  *  *  *  *
    //       *-------------------D-------------------D
    //       *                   *                   *
    //       *                   *                   *
    //       *                   *                   *
    //       *-------------------*-------------------B

    // 找到全部点的包围盒，之后在新的三个点，ABC，如果不想重复的话，可以稍微增加一点大小，
    // 用来保证不会有点出现在这个这个边上
    // 然后再将这三个点创建出一个三角形
    //
    //       A  * * * * * * * * * * * * * C             // 用三角形来表示是没有问题的
    //          *                         *             // 问题在哪里呢？
    //            *                       *             // 第一个应该是定位的问题
    //              *                     *             // 我应该知道我下一个想要添加的点在那个三角形里面
    //                *                   *             // 如果拿到这个三角形的hf的结构
    //                  *                 *             // 然后是如何给这个hf结构中添加一个点
    //                    *               *             // 第一步是如何找到这个face
    //                      *             *             // 找并不是问题，大不了直接暴力查找就好
    //                        *           *             // 再之后就是添加的问题
    //                          *         *             // 添加也不算问题，画一个图，
    //                            *       *             // 添加一个函数，在一个face中添加一个点
    //                              *     *             // 并将其他点与其全部连接起来就好
    //                                *   *             // 在设置好三个面就行
    //                                  * *             // 其实还有一个问题，一条边有正反两个边，有没有办法
    //                                    B             // 恒定的确定其中一条边的序号为奇数
    // 可能会出现三角形内                                  // 也许有，但是目前我还没有想到
    //       A  * * * * * * * * * * * * * C             // 这里会有一个比较固定的方法
    //          *   *                   * *             // 需要去画图，写一个新的方法
    //            *      *            *   *             // 在一个face中添加一个点
    //              *         *     *     *             // 三角形的三个点是逆时针排列的
    //                *           D       *             // 
    //                  *                 *             //
    //                    *         *     *             //
    //                      *             *             //
    //                        *       *   *             //
    //                          *         *             //
    //                            *    *  *             //
    //                              *     *             //
    //                                * * *             //
    //                                  * *             //
    //                                    B             //
    // 还有可能会出现在其边上,也有可能会出现在另一条边上        //
    //       A  * * * * * * * * * * * * * C             // 这里需要添加一个判断条件
    //          *                      *  *             // 比如对面的face是不是也是三角形
    //            *                 *     *             // 如果不是的话，就不能执行这个操作了
    //              *            *        *             // 应该就只会在当前边上进行操作
    //                *        *          *             // 这个应该是有一个直接的操作能够实现的
    //                  *   *             *             // 这里完成了，
    //                    D               *             //
    //                      *             *             //
    //                        *           *             //
    //                          *         *             //
    //                            *       *             //
    //                              *     *             //
    //                                *   *             //
    //                                  * *             //
    //                                    B             //
    //
    //       A  * * * * * * * * * * * * * C             // 找到这条边
    //          *                      *  *             // 找到另外两个三角形
    //        *   *                 *     *             // 然后先将边劈开
    //              *            *        *             // 再把三个face的边的前后顺序搞定
    //         *      *        *          *             // 再把三个face的索引更新正确
    //                  *   *             *             //
    //         *          D               *             //
    //                   *  *             *             //
    //          *       *     *           *             //
    //                 *        *         *             //
    //          *     *           *       *             //
    //               *              *     *             //
    //           *  *                 *   *             //
    //             *                    * *             //
    //            E * * * * * * * * * * * B             //

    // 再之后需要做什么操作呢？
    // 检查边，查看是否需要flip

    Triangle_node<Point_2> *make_Triangle_node(half_edge_struct<vertex_xy> *hf, int face_index) {
        auto temp_flag = hf->get_triangle_face_vertex(face_index);
        auto new_triangle_node = new Triangle_node<Point_2>(temp_flag.a, temp_flag.b, temp_flag.c, face_index);
        return new_triangle_node;
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
    bool legalize_edge(half_edge_struct<vertex_xy> *hf, half_edge_index half_edge_index, vertex_index vertex_index,
                       Triangle_node_tree<Point_2> *tree) {
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
        auto face_index = hf->get_face_index(half_edge_index);
        auto all_edges_of_first_face = hf->get_all_edge_of_face(half_edge_index);
        auto all_edges_of_second_face = hf->get_all_edge_of_face(hf->get_opposite_edge_index(half_edge_index));
        // 下面的判断里面少了一步确定非凹，两个三角形组成了一个凹四边形 todo:
        if (all_edges_of_first_face.size() == 3 &&
            all_edges_of_second_face.size() == 3 &&
            hf->if_convex_quadrangle(half_edge_index) &&
            hf->get_face(hf->get_edge(hf->get_opposite_edge_index(half_edge_index)).incident_face).boundary_type !=
            Face::BOUNDARY_TYPE::hole_face) {
            //    B----------D
            //    *  *       *
            //    *    *     *
            //    *      *   *
            //    *        * *
            //    A----------C
            //  BC 两个点相互交换应该是没有问题的
            auto BC_edge = half_edge_index;

            auto AC_or_AB_edge = hf->get_pre_edge_index(half_edge_index);
            auto AB_or_AC_edge = hf->get_next_edge_index(half_edge_index);

            // 确定是 AC_or_AB 而不是 DB_or_DC
            if (hf->get_edge(AC_or_AB_edge).vertex_index == vertex_index ||
                hf->get_edge(AB_or_AC_edge).vertex_index == vertex_index) {
                AC_or_AB_edge = hf->get_pre_edge_index(hf->get_opposite_edge_index(half_edge_index));
                AB_or_AC_edge = hf->get_next_edge_index(hf->get_opposite_edge_index(half_edge_index));
            }
            auto A_point = hf->get_vertex(AC_or_AB_edge);
            auto B_point = hf->get_vertex(half_edge_index);
            auto D_point = hf->vertices.at(vertex_index);
            auto C_point = hf->get_vertex(hf->get_opposite_edge_index(half_edge_index));
            //

            auto centre = Point_2::centre_of_a_circle(A_point, B_point, C_point);
            if (Point_2::distance_compare(D_point - centre, C_point - centre)) {
                // 当前是合法的
            } else {
                // 当前是非法的，需要执行flip操作，将原本的BC边切换为AC边
                Triangle_node<Point_2> *triangle_node;
                Triangle_node<Point_2> *triangle_node_2;
                if (tree != nullptr) {
                    auto centroid = hf->get_centroid(hf->get_face_index(half_edge_index));
                    auto centroid_2 = hf->
                            get_centroid(hf->get_face_index(hf->get_opposite_edge_index(half_edge_index)));
                    triangle_node = tree->find_triangle_node(centroid);
                    triangle_node_2 = tree->find_triangle_node(centroid_2);
                }
                hf->flip_edge(half_edge_index);
                if (tree != nullptr) {
                    auto new_triangle_node = make_Triangle_node(hf, hf->get_face_index(half_edge_index));
                    auto new_2_triangle_node = make_Triangle_node(hf,
                                                                  hf->get_face_index(
                                                                      hf->get_opposite_edge_index(half_edge_index)));
                    triangle_node->add_triangle_node(new_triangle_node);
                    triangle_node->add_triangle_node(new_2_triangle_node);
                    triangle_node_2->add_triangle_node(new_triangle_node);
                    triangle_node_2->add_triangle_node(new_2_triangle_node);
                }
                // half_edge_index 这个索引并没有改变
                // 但是边需要变更了，需要变更为 AB 或者 AC ，但是不能是 BD 或者 CD ,前面添加条件确定了
                legalize_edge(hf, AC_or_AB_edge, vertex_index, tree);
                legalize_edge(hf, AB_or_AC_edge, vertex_index, tree);
            }
        }
        return false;
    }


    Half_edge *delaunay_triangulation(std::vector<Point_2> input_points) {
        auto box = AABB_min_max<Point_2>(input_points);
        auto point_a = box.max_point + (box.max_point - box.min_point);
        Point_2 point_b = {box.max_point.x, box.min_point.y - (box.max_point.y - box.min_point.y)};
        Point_2 point_c = {box.min_point.x - (box.max_point.x - box.min_point.x), box.max_point.y};
        auto hf = new half_edge_struct<vertex_xy>;
        auto ab_index = hf->add_triangle(point_a, point_b, point_c);
        auto abc_face_index = hf->get_face_index(ab_index);
        auto root_node = new Triangle_node<Point_2>(point_a, point_b, point_c, abc_face_index);

        auto tree = new Triangle_node_tree<Point_2>(root_node);


        for (auto point: input_points) {
            face_index result_face_index = 0;
            half_edge_index edge_index = 0;
            auto triangle_node = tree->find_triangle_node(point); // 拿到结点
            // triangle_node只负责给出face_index,再去判断这个点是在三角形的边上还是内部(half_edge 给出)
            // 给出结果，相应结束的话，可以很快的结束这个问题
            if (triangle_node != nullptr) {
                result_face_index = triangle_node->face_index;
            }
            auto type_temp = hf->if_vertex_in_face(point,
                                                    hf->get_face_incident_edge(result_face_index),
                                                    edge_index);


            // auto type_temp = hf->get_vertex_in_which_face_for_test(result_face_index, edge_index, point);
            // 上面这一行的参数不用改，不，参数也是需要改的，主要是为了进行加速
            // 需要一个三角形的结构，需要同步在下面add_new_point 的时候，添加在树的结构中相同的内容
            // 在insert_edge的时候，将原本的一条边上的两个面，劈成四个三角形
            // 不对，还没有结束，后面的对角线变更的时候，还会改变三角形，
            // 这时候拿到的face或者edge就不准了，要么变更树，要么就要使用之前的点定位的办法了
            // 再看书时候发现，flip的时候也是需要去更改查找树的。
            // 我暂时不想这个问题太负责，就先不继续去做了。
            // 继续把这个问题做下去，先简单的改一改。
            //

            if (type_temp == point_in_triangle_type::in_triangle) {
                // 上面的函数并没有考虑另一种情况，那就是在边上的情况
                // 上面的在边上的情况会返回负一，之后怎么对这个负一进行处理，或者说怎么得到在那条边上的情况
                // 先不考虑在边上的情况，之后考虑什么呢？
                int vertex_index = 0;
                auto centroid = hf->get_centroid(result_face_index);
                auto triangle_node = tree->find_triangle_node(centroid);
                auto new_faces = hf->face_add_new_point(result_face_index, point, vertex_index);
                for (auto face: new_faces) {
                    std::vector<Triangle<Point_2> > result_segments{};
                    auto new_triangle_node = make_Triangle_node(hf, face);
                    triangle_node->add_triangle_node(new_triangle_node);

                    auto temp = hf->get_ab_edge_from_face_abc(face, vertex_index);
                    legalize_edge(hf, temp, vertex_index, tree);
                    // 有问题，运行的时候发生了死循环
                }
            } else if (type_temp == point_in_triangle_type::on_edge) {
                // 在AB边上添加一个点，现在这个边变成了ADB，返回DB这条边
                auto ab_edge_index = edge_index;
                auto ba_edge_index = hf->get_opposite_edge_index(ab_edge_index);
                auto centroid = hf->get_centroid(hf->get_face_index(ab_edge_index));
                auto centroid_2 = hf->get_centroid(hf->get_face_index(ba_edge_index));
                auto triangle_node = tree->find_triangle_node(centroid);
                auto triangle_node_2 = tree->find_triangle_node(centroid_2);

                auto db_edge_index = hf->insert_edge(ab_edge_index, point);
                auto bd_edge_index = hf->get_opposite_edge_index(db_edge_index);

                ear_clip_triangulations(*hf, db_edge_index);
                ear_clip_triangulations(*hf, hf->get_opposite_edge_index(db_edge_index)); {
                    auto new_triangle_node = make_Triangle_node(hf, hf->get_face_index(ab_edge_index));
                    auto new_2_triangle_node = make_Triangle_node(hf, hf->get_face_index(db_edge_index));
                    triangle_node->add_triangle_node(new_triangle_node);
                    triangle_node->add_triangle_node(new_2_triangle_node);
                } {
                    auto new_triangle_node = make_Triangle_node(hf, hf->get_face_index(ba_edge_index));
                    auto new_2_triangle_node = make_Triangle_node(hf, hf->get_face_index(bd_edge_index));
                    triangle_node_2->add_triangle_node(new_triangle_node);
                    triangle_node_2->add_triangle_node(new_2_triangle_node);
                }

                // 有了插入的一条边，然后呢？需要做什么呢？
                // 找到新的两个点，将原本的一个三角形分成两个三角形,需要调用之前完成的三角化的算法
                // 需要找到这个顶点的全部的face,原本有两个面，劈开之后应该是有四个面的
                // 四个face 再去做 是否合法的测试
                auto vertex_index = hf->get_edge(db_edge_index).vertex_index;
                auto all_face_from_one_vertex = hf->get_all_face_of_vertex(vertex_index);
                // 拿到的面的数量是不够的,应该是1，4，0，5的，但是目前数量不够
                for (auto face: all_face_from_one_vertex) {
                    auto temp = hf->get_ab_edge_from_face_abc(face, vertex_index);
                    legalize_edge(hf, temp, vertex_index, tree);
                }
            }
        }
    }
};


#endif //TEST_DELAUNAY_TRIANGULATION_H
