//
// Created by 潘鑫 on 2025/10/8.
//

#include <random>
#include <gtest/gtest.h>

#include "base_element/geometry/triangle.h"
#include "ear_clip.h"
#include "labyrinth.h"
#include "observer_manage.h"
#include "base_element/half_edge/half_edge_struct.h"
#include "trapezoid_graph.h"
#include "tree_function.h"
#include "base_event.h"
#include "base_observer.h"

#include "render_object_manage.h"
#include "windows.h"


template<typename T>
T &init_hf(T &hf) {
    auto half_edge_index = hf.create_loop({0, 5}, {-2, 3});
    auto first_half_edge = half_edge_index;
    half_edge_index = hf.add_edge(half_edge_index, {0, 0});
    half_edge_index = hf.insert_edge(half_edge_index, {3, 2});
    half_edge_index = hf.insert_edge(half_edge_index, {5, 1});
    half_edge_index = hf.insert_edge(half_edge_index, {7, 2});
    half_edge_index = hf.insert_edge(half_edge_index, {5, 3});
    half_edge_index = hf.insert_edge(half_edge_index, {3, 3});
    half_edge_index = hf.insert_edge(half_edge_index, {2, 5});
    half_edge_index = hf.insert_edge(half_edge_index, {1, 2});
    half_edge_index = hf.insert_edge(half_edge_index, {-1, 3});
    return hf;
}


template<typename T>
T &init_hf_2(T &hf) {
    auto half_edge_index = hf.create_loop({3, 2}, {5, 1});
    auto first_half_edge = half_edge_index;
    half_edge_index = hf.add_edge(half_edge_index, {7, 2});
    half_edge_index = hf.insert_edge(half_edge_index, {5, 3});
    return hf;
}

template<typename T>
T &init_expect_triangles(T &expect_triangles) {
    expect_triangles.push_back(Triangle<Point_2>{{0, 0}, {1, 2}, {-1, 3}});
    expect_triangles.push_back(Triangle<Point_2>{{3, 2}, {5, 1}, {7, 2}});
    expect_triangles.push_back(Triangle<Point_2>{{3, 2}, {7, 2}, {5, 3}});
    expect_triangles.push_back(Triangle<Point_2>{{3, 2}, {5, 3}, {3, 3}});
    expect_triangles.push_back(Triangle<Point_2>{{3, 2}, {3, 3}, {2, 5}});
    expect_triangles.push_back(Triangle<Point_2>{{3, 2}, {2, 5}, {1, 2}});
    expect_triangles.push_back(Triangle<Point_2>{{-1, 3}, {0, 5}, {-2, 3}});
    expect_triangles.push_back(Triangle<Point_2>{{-1, 3}, {-2, 3}, {0, 0}});
    expect_triangles.push_back(Triangle<Point_2>{{0, 0}, {3, 2}, {1, 2}});
    return expect_triangles;
}


TEST(ear_clip, from_half_edge_create_loop_vertices) {
    half_edge_struct<vertex_xy> hf{};
    hf = init_hf(hf);
    Face temp;
    hf.get_first_face(temp);
    auto all_edge = hf.get_all_edge_of_face(hf.get_pre_edge_index(temp.bounding_half_edge));
    auto new_segments = hf.get_vertices(all_edge);

    std::vector<Point_2> segments{};
    segments.push_back(Point_2{-2, 3});
    segments.push_back(Point_2{0, 0});
    segments.push_back(Point_2{3, 2});
    segments.push_back(Point_2{5, 1});
    segments.push_back(Point_2{7, 2});
    segments.push_back(Point_2{5, 3});
    segments.push_back(Point_2{3, 3});
    segments.push_back(Point_2{2, 5});
    segments.push_back(Point_2{1, 2});
    segments.push_back(Point_2{-1, 3});
    segments.push_back(Point_2{0, 5});

    if (segments.size() == new_segments.size()) {
        for (int i = 0; i < new_segments.size(); ++i) {
            EXPECT_EQ(new_segments.at(i), segments.at(i)) << "i value: " << i
        << std::endl;
            // 这里的打印也很方便，不出现错误的时候是不需要打印的
        }
    } else {
        FAIL() << " segments.size() == new_segments.size()" << std::endl;
    }
}

TEST(ear_clip, ear_clip) {
    // 既然是测试，那么需要测试输入有什么？
    // 输入就需要一个half-edge 的结构，
    // 然后，可以是另一个结构的，比如什么呢？
    // 一系列的点，默认他们之间存在边的连接的线
    // 既然有了输入，那么输出呢？
    // 一堆的三角形
    // 三角形应该是什么结构呢？
    // 三个点组成的三角形，可以的，但是之后还需要改
    // 为什么还需要改呢？
    // 因为之后还需要按照顶点和索引的方式来确定最后给出到vulkan或者OpenGL的结构
    // 那么先不管这些，第一个问题是，如果出现了一个凹点的时候应该怎么解决呢？
    //


    half_edge_struct<vertex_xy> hf{};
    hf = init_hf(hf);
    Face temp;
    hf.get_first_face(temp);
    auto all_edge = hf.get_all_edge_of_face(hf.get_pre_edge_index(temp.bounding_half_edge));
    auto new_segments = hf.get_vertices(all_edge);

    RB_Tree_Node<Point_2> *tree_vertices = nullptr;
    for (auto new_segment: new_segments) {
        tree_vertices = tree_vertices->tree_insert_value(tree_vertices, new_segment);
    }

    std::vector<Triangle<Point_2> > expect_triangles{};
    std::vector<Triangle<Point_2> > result_segments{};
    expect_triangles.push_back(Triangle<Point_2>{{3, 2}, {5, 3}, {3, 3}});
    expect_triangles.push_back(Triangle<Point_2>{{3, 2}, {7, 2}, {5, 3}});
    expect_triangles.push_back(Triangle<Point_2>{{0, 0}, {1, 2}, {-1, 3}});
    expect_triangles.push_back(Triangle<Point_2>{{3, 2}, {5, 1}, {7, 2}});
    expect_triangles.push_back(Triangle<Point_2>{{3, 2}, {2, 5}, {1, 2}});
    expect_triangles.push_back(Triangle<Point_2>{{0, 0}, {3, 2}, {1, 2}});
    expect_triangles.push_back(Triangle<Point_2>{{3, 2}, {3, 3}, {2, 5}});
    expect_triangles.push_back(Triangle<Point_2>{{-1, 3}, {0, 5}, {-2, 3}});
    expect_triangles.push_back(Triangle<Point_2>{{-1, 3}, {-2, 3}, {0, 0}});


    std::sort(expect_triangles.begin(), expect_triangles.end(), std::less<>());


    if (ear_clip_algorithm_no_efficient(result_segments, new_segments, *tree_vertices) == true &&
        result_segments.size() == expect_triangles.size()) {
        std::sort(result_segments.begin(), result_segments.end(), std::less<>());
        for (int i = 0; i < result_segments.size(); ++i) {
            EXPECT_EQ(result_segments.at(i)==expect_triangles.at(i), true) << "i value: " << i << std::endl;
            // 这里的打印也很方便，不出现错误的时候是不需要打印的
        }
    } else {
        FAIL() << "ear_clip_algorithm_no_efficient failed "
        << " or  result_segments.size() != expect_triangles.size()" << std::endl;
    }
    // 拿到了正确的输入的结果，只不过是强行拿到的，并不是自己手动计算处理的，所以结果必然是正确的
}


TEST(ear_clip, ear_clip_half_edge) {
    // 既然是测试，那么需要测试输入有什么？
    // 输入就需要一个half-edge 的结构，
    // 然后，可以是另一个结构的，比如什么呢？
    // 一系列的点，默认他们之间存在边的连接的线
    // 既然有了输入，那么输出呢？
    // 一堆的三角形
    // 三角形应该是什么结构呢？
    // 三个点组成的三角形，可以的，但是之后还需要改
    // 为什么还需要改呢？
    // 因为之后还需要按照顶点和索引的方式来确定最后给出到vulkan或者OpenGL的结构
    // 那么先不管这些，第一个问题是，如果出现了一个凹点的时候应该怎么解决呢？
    //


    half_edge_struct<vertex_xy> hf{};
    hf = init_hf(hf);
    Face temp;
    hf.get_first_face(temp);
    auto all_edge = hf.get_all_edge_of_face(hf.get_pre_edge_index(temp.bounding_half_edge));
    auto new_segments = hf.get_vertices(all_edge);

    RB_Tree_Node<Point_2> *tree_vertices = nullptr;
    for (auto new_segment: new_segments) {
        tree_vertices = tree_vertices->tree_insert_value(tree_vertices, new_segment);
    }

    std::vector<Triangle<Point_2> > expect_triangles{};
    init_expect_triangles(expect_triangles);

    if (ear_clip_algorithm_half_edge(hf, all_edge, *tree_vertices) == true) {
        auto result_segments = hf.get_all_triangles_data(true);

        auto labyrinth = new Labyrinth("迷宫");

        auto temp_trapezoid = new render_component(nullptr, "ear clip 后的三角形组");
        /***************设置参数**********************/
        std::vector<VertexAttrib> vertex_attribs;
        vertex_attribs.emplace_back(3,GL_FLOAT,GL_FALSE, sizeof(Point_3), (void *) 0);
        // vertex_attribs.emplace_back(3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) (3 * sizeof(float)));

        std::vector<Point_3> vertices;
        std::vector<unsigned int> indices;
        for (auto result_segment: result_segments) {
            indices.push_back(vertices.size() + 0);
            indices.push_back(vertices.size() + 1);
            indices.push_back(vertices.size() + 2);
            vertices.emplace_back(result_segment.a);
            vertices.emplace_back(result_segment.b);
            vertices.emplace_back(result_segment.c);
        }
        // 参数这里最重要的是下面的两行
        temp_trapezoid->set_VBO_parameter(vertices.size() * sizeof(Point_3), vertices.data(), vertex_attribs);
        temp_trapezoid->set_EBO_parameter(indices.size() * sizeof(GLuint), indices.data(), indices.size());
        temp_trapezoid->set_vertex_shader("render/shader/different_color.vert");
        temp_trapezoid->set_fragment_shader("render/shader/different_color.frag");

        /***************添加到渲染管理器**********************/
        add_object_to_render_manager(temp_trapezoid);

        add_render_windows();

        delete labyrinth;


        if (result_segments.size() == expect_triangles.size()) {
            for (int i = 0; i < result_segments.size(); ++i) {
                EXPECT_EQ(result_segments.at(i)== expect_triangles.at(i), true) << "i value: " << i << std::endl;
                // 这里的打印也很方便，不出现错误的时候是不需要打印的
                // 比较还是有点问题，两个向量对比需要重新写一个函数，因为它们的排序可能会不一致
            }
        } else {
            FAIL() << "ear_clip_algorithm_no_efficient failed "
            << " or  result_segments.size() != expect_triangles.size()" << std::endl;
        }
    } else {
        FAIL() << "ear_clip_algorithm_half_edge return false " << std::endl;
    }
    // 拿到了正确的输入的结果，只不过是强行拿到的，并不是自己手动计算处理的，所以结果必然是正确的
}

// 如果是不带洞的，那么直接用是没有问题的，带洞的话，就稍微有点问题，不是论文中提到的办法能够直接解决的了
// 第一件事是三角形的划分结果肯定是对的，那么问题在哪里？


TEST(half_edge, test_flip_edge) {
    half_edge_struct<vertex_xy> hf{};
    hf = init_hf_2(hf);
    Face temp;
    hf.get_first_face(temp);
    auto all_edge = hf.get_all_edge_of_face(hf.get_pre_edge_index(temp.bounding_half_edge));
    auto new_segments = hf.get_vertices(all_edge);

    RB_Tree_Node<Point_2> *tree_vertices = nullptr;
    for (auto new_segment: new_segments) {
        tree_vertices = tree_vertices->tree_insert_value(tree_vertices, new_segment);
    }

    std::vector<Triangle<Point_2> > expect_triangles{};
    init_expect_triangles(expect_triangles);

    if (ear_clip_algorithm_half_edge(hf, all_edge, *tree_vertices) == true) {
    }
    hf.flip_edge(8);
    int a = 90;
}

TEST(half_edge, test_face_and_point) {
    half_edge_struct<vertex_xy> hf{};
    auto half_edge_index = hf.create_loop({7, 8}, {12, 8});
    auto first_half_edge = half_edge_index;
    half_edge_index = hf.add_edge(half_edge_index, {10, 3});
    int vertex_index = 0;
    hf.face_add_new_point(1, {10, 6}, vertex_index);
    int a = 90;
}


void test_point_location(half_edge_struct<vertex_xy> &hf, trapezoid_graph_Node<int> *root,
                         Point_2 find_point) {
    face_index result_face_index = 0;
    half_edge_index edge_index = 0;
    if (point_in_triangle_type::out_triangle ==
        hf.get_vertex_in_which_face_for_test(result_face_index, edge_index, find_point)) {
        result_face_index = -1;
    }
    auto start_point_trapezoid = trapezoid_graph_Node<int>::find_point_in_trapezoid_graph(root, find_point);
    if (start_point_trapezoid == nullptr) {
        GTEST_FAIL() << "start_point_trapezoid == nullptr " << find_point << std::endl;
    }
    if (hf.faces.at(start_point_trapezoid->data).boundary_type == Face::BOUNDARY_TYPE::hole_face
        && result_face_index == -1) {
        GTEST_SUCCEED();
    } else
        EXPECT_EQ(result_face_index, start_point_trapezoid->data) << find_point;
}

TEST(ear_clip, test_point_location) {
    half_edge_struct<vertex_xy> hf{};
    hf = init_hf(hf);
    Face temp;
    hf.get_first_face(temp);
    auto all_edge = hf.get_all_edge_of_face(hf.get_pre_edge_index(temp.bounding_half_edge));
    auto new_segments = hf.get_vertices(all_edge);

    RB_Tree_Node<Point_2> *tree_vertices = nullptr;
    for (auto new_segment: new_segments) {
        tree_vertices = tree_vertices->tree_insert_value(tree_vertices, new_segment);
    }


    if (ear_clip_algorithm_half_edge(hf, all_edge, *tree_vertices) == true) {
        // 这里是进行分解完之后，那么需要先确定每个三角形对应的面的索引，也就是在那个索引中
        face_index result_face_index = 0;
        half_edge_index edge_index = 0;
        hf.get_vertex_in_which_face_for_test(result_face_index, edge_index, {1, 1});
        EXPECT_EQ(9, result_face_index); // 原因是出现了 on_edge,但是没有看是否在线段范围内
        hf.get_vertex_in_which_face_for_test(result_face_index, edge_index, {5, 1.1});
        EXPECT_EQ(2, result_face_index);
        hf.get_vertex_in_which_face_for_test(result_face_index, edge_index, {5, 2.5});
        EXPECT_EQ(3, result_face_index);
        hf.get_vertex_in_which_face_for_test(result_face_index, edge_index, {3.1, 2.9});
        EXPECT_EQ(4, result_face_index);
        hf.get_vertex_in_which_face_for_test(result_face_index, edge_index, {2.9, 3});
        EXPECT_EQ(5, result_face_index);
        hf.get_vertex_in_which_face_for_test(result_face_index, edge_index, {2, 3});
        EXPECT_EQ(6, result_face_index);
        hf.get_vertex_in_which_face_for_test(result_face_index, edge_index, {-1, 3.1});
        EXPECT_EQ(7, result_face_index);
        hf.get_vertex_in_which_face_for_test(result_face_index, edge_index, {-1, 2});
        EXPECT_EQ(8, result_face_index);

        auto result_segments = hf.get_all_triangles_data(true);
        if (result_segments.empty() == false) {
            std::random_device rd;
            std::mt19937 g(rd());
            // obtain a time-based seed:
            // unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
            // shuffle (foo.begin(), foo.end(), std::default_random_engine(seed));
            // 其中注释的是另一种实现办法

            // 我需要拿到 half_edge 结构中全部的线段

            std::shuffle(result_segments.begin(), result_segments.end(), g);
        }

        auto bounding_box = hf.calculate_aabb();
        auto root = trapezoid_graph_Node<int>::init_root(bounding_box);
        for (int i = 0; i < hf.half_edges.size(); ++i, ++i) {
            root = trapezoid_graph_Node<int>::add_a_segment(root, &hf, i);
            auto result = find_all_leaf_node(root);
            // std::cout << "result" << result->size() << std::endl;
        }
        auto result_2 = find_all_leaf_node(root);


        auto temp_trapezoid = new render_component(nullptr, "梯形");
        /***************设置参数**********************/
        std::vector<VertexAttrib> vertex_attribs;
        vertex_attribs.emplace_back(3,GL_FLOAT,GL_FALSE, sizeof(Point_3), (void *) 0);
        // vertex_attribs.emplace_back(3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) (3 * sizeof(float)));

        std::vector<Point_3> vertices;
        std::vector<unsigned int> indices;
        for (auto result_segment: *result_2) {
            indices.push_back(vertices.size() + 0);
            indices.push_back(vertices.size() + 1);
            indices.push_back(vertices.size() + 2);
            indices.push_back(vertices.size() + 2);
            indices.push_back(vertices.size() + 3);
            indices.push_back(vertices.size() + 0);
            vertices.emplace_back(result_segment->trapezoid_union_data.trapezoid.left_lower); //0 1 2
            vertices.emplace_back(result_segment->trapezoid_union_data.trapezoid.right_lower);
            vertices.emplace_back(result_segment->trapezoid_union_data.trapezoid.right_upper); // 2 3 0
            vertices.emplace_back(result_segment->trapezoid_union_data.trapezoid.left_upper);
        }
        // 参数这里最重要的是下面的两行
        temp_trapezoid->set_VBO_parameter(vertices.size() * sizeof(Point_3), vertices.data(), vertex_attribs);
        temp_trapezoid->set_EBO_parameter(indices.size() * sizeof(GLuint), indices.data(), indices.size());
        temp_trapezoid->set_vertex_shader("render/shader/different_color.vert");
        temp_trapezoid->set_fragment_shader("render/shader/different_color.frag");

        /***************添加到渲染管理器**********************/
        add_object_to_render_manager(temp_trapezoid);

        add_render_windows();


        test_point_location(hf, root, {1.1, 1.1});
        test_point_location(hf, root, {1, 1});
        test_point_location(hf, root, {5, 1.1});
        test_point_location(hf, root, {5, 2.5});
        test_point_location(hf, root, {3.2, 2.9});
        test_point_location(hf, root, {2.9, 3});
        test_point_location(hf, root, {2, 3});
        test_point_location(hf, root, {-1, 3.1});
        test_point_location(hf, root, {-1, 2});
        test_point_location(hf, root, {2.9, 3});
        test_point_location(hf, root, {1, 4});
    } else {
        FAIL() << "ear_clip_algorithm_half_edge return false " << std::endl;
    }
}


// 下面是关于两个不同的智能指针的简单测试
// 树应该是不太好使用智能指针的
// std::shared_ptr侵占独占指针所指向的对象的独占权，所以独占指针被设置为null
std::unique_ptr<int> set(std::unique_ptr<int> &&tem) {
    *tem = 5;
    return tem;
}

std::shared_ptr<int> set(std::shared_ptr<int> tem) {
    *tem = 5;
    return tem;
}

TEST(unique_point, int) {
    std::unique_ptr<int> p{new int(3)};
    auto new_p = set(std::move(p));
    EXPECT_EQ(*new_p, 5);
    EXPECT_EQ(p, nullptr);
    std::move(new_p);                                 // 只调用一个单独的move是没有什么作用的
    std::shared_ptr<int> shared_p = std::move(new_p); // 还需要添加等号或者转移的实际操作才会生效
    EXPECT_EQ(*shared_p, 5);
}


TEST(share_point, int) {
    std::shared_ptr<int> p{new int(3)};
    auto new_p = set(p);
    EXPECT_EQ(*new_p, 5);
    EXPECT_NE(p, nullptr);
}
