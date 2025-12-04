//
// Created by 潘鑫 on 2025/11/12.
//

#ifndef TEST_DELAUNAY_TRIANGULATION_H
#define TEST_DELAUNAY_TRIANGULATION_H
#include <glm/fwd.hpp>
#include "ear_clip.h"
#include "half_edge.h"
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


    Half_edge *delaunay_triangulation(std::vector<point_2> input_points) {
        auto box = AABB<point_2>::calculate_bound_box(input_points);
        auto point_a = box.max_point + (box.max_point - box.min_point);
        point_2 point_b = {box.max_point.x, box.min_point.y - (box.max_point.y - box.min_point.y)};
        point_2 point_c = {box.min_point.x - (box.max_point.x - box.min_point.x), box.max_point.y};
        auto hf = new half_edge_struct<vertex_xy>;
        auto ab_index = hf->add_triangle(point_a, point_b, point_c);
        auto abc_face_index = hf->get_face_index(ab_index);
        auto root_node = new Triangle_node<point_2>(point_a, point_b, point_c, abc_face_index);

        auto tree = new Triangle_node_tree<point_2>(root_node);


        for (auto point: input_points) {
            Face_v_index result_face_index = 0;
            Half_edge_v_index edge_index = 0;
            auto type_temp = hf->get_vertex_in_which_face_for_test(result_face_index, edge_index, point);
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
                    std::vector<Triangle<point_2> > result_segments{};
                    auto new_triangle_node = hf->make_Triangle_node(face);
                    triangle_node->add_triangle_node(new_triangle_node);

                    auto temp = hf->get_ab_edge_from_face_abc(face, vertex_index);
                    hf->legalize_edge(temp, vertex_index, tree);
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
                    auto new_triangle_node = hf->make_Triangle_node(hf->get_face_index(ab_edge_index));
                    auto new_2_triangle_node = hf->make_Triangle_node(hf->get_face_index(db_edge_index));
                    triangle_node->add_triangle_node(new_triangle_node);
                    triangle_node->add_triangle_node(new_2_triangle_node);
                } {
                    auto new_triangle_node = hf->make_Triangle_node(hf->get_face_index(ba_edge_index));
                    auto new_2_triangle_node = hf->make_Triangle_node(hf->get_face_index(bd_edge_index));
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
                    hf->legalize_edge(temp, vertex_index, tree);
                }
            }
        }
    }
};


#endif //TEST_DELAUNAY_TRIANGULATION_H
