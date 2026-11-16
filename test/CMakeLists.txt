cmake_minimum_required(VERSION 3.16)
project(google_test LANGUAGES CXX)


add_executable(hello_test ${LABYRINTH}
        #        spherical_harmonics/spherical_harmonics_test.cpp
        add_char_calculata.cpp
        test_openvdb.cpp
        test_add_char_calculata.cpp
        the_centre_of_a_circle.cpp
        segment_end_ponit_and_gradient.cpp
        longest_continuous_sequence.cpp
        delaunay_triangulation.cpp
        test_tetrahedron.cpp
        convex_hull.cpp
        bracket.cpp
        test_QEM.cpp
        test_factory.cpp
        cut_rod.cpp
        test_tree_node.cpp
        test_class.cpp
        test_SH.cpp
        knapsack.cpp
        test_template.cpp
        test_plane_sweep.cpp
        ear_clip.cpp
        test_Tent_Filter.cpp
        test_ear_clip.cpp
        test_mutex.cpp
        test_best-fit.cpp
        test_occt.cpp
        test_quadtree.cpp
        test_particle.cpp
        test_intersect.cpp
        test_reader_writer_queue.cpp
        test_lazy_Evaluation.cpp
        test_signed_area.cpp
        test_conveyor_belt.cpp
        test_cut_rod.cpp)


enable_testing()

include(GoogleTest)

add_executable(cache_time hello_test.cpp)
gtest_discover_tests(cache_time)

gtest_discover_tests(hello_test DISCOVERY_TIMEOUT 30)
target_compile_definitions(hello_test PRIVATE VK_NO_PROTOTYPES)

target_link_libraries(hello_test PRIVATE ccd)
#target_link_libraries(hello_test PRIVATE hwy)
target_link_libraries(hello_test PRIVATE readerwriterqueue)
target_link_libraries(hello_test PRIVATE OpenVDB::openvdb)
target_link_libraries(hello_test PRIVATE GTest::gtest_main)
target_link_libraries(hello_test PRIVATE TBB::tbb)
target_link_libraries(hello_test PRIVATE Eigen3::Eigen)
target_link_libraries(hello_test PRIVATE EnTT::EnTT)
target_link_libraries(hello_test PRIVATE quill::quill)
target_link_libraries(hello_test PRIVATE project_include)

target_compile_options(hello_test PRIVATE -g)
#target_compile_options(hello_test PRIVATE -fsanitize=thread) // 需要开另一个 gtest 程序，不能和另外两个一起使用
target_compile_options(hello_test PRIVATE -fno-omit-frame-pointer)
#target_compile_options(hello_test PRIVATE -fsanitize=address,undefined)
#target_link_options(hello_test PRIVATE -fsanitize=address,undefined)

# mac 上不支持leak ,在linux上是可以开启的, mac 上 Instruments，选择 Allocations 或 Leaks 模板，直接录制你的可执行程序
#target_compile_options(hello_test PRIVATE -fsanitize=address,undefined,leak)
#target_link_options(hello_test PRIVATE -fsanitize=address,undefined,leak)

target_compile_definitions(hello_test PRIVATE NDEBUG)

add_executable(test_mutex
        #        test_curve.cpp
        test_thread_fibonacci.cpp
        test_dot_product.cpp
        test_matrix_view_model_projection.cpp
        test_mutex.cpp)
gtest_discover_tests(test_mutex)
target_link_libraries(test_mutex GTest::gtest_main)

add_executable(fixture sample3.cpp)
gtest_discover_tests(fixture)
target_link_libraries(fixture GTest::gtest_main project_include)

add_executable(hello_mac
        #        run_time.cpp
        make_heap.cpp
        thread.cpp
        cut_rod.cpp
        run_priority_queue.cpp
        run_queue.cpp
        longest_continuous_sequence.cpp
        set_graph.cpp)
target_link_libraries(hello_mac ${CURSES_LIBRARIES} EnTT::EnTT quill::quill)
