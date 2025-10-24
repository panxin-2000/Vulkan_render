cmake_minimum_required(VERSION 3.27)
project(hello_mac)

set(CMAKE_CXX_STANDARD 17)


find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -save-temps=obj")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -save-temps=obj")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
add_compile_options(-fno-omit-frame-pointer)
add_compile_options(-mno-omit-leaf-frame-pointer)

## 按照googletest官网文档添加了gtest
include(FetchContent)

## 官网给的FetchContent_Declare如下，但是不方便直观的看明白版本
#FetchContent_Declare(
#        googletest
#        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
#)
## 我修改如下，方便看清楚版本号
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.11.0
)

## 下面这行没有看懂什么意思，按照说明先复制过来了
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
#FetchContent_MakeAvailable() 命令会确保指定的依赖项已被填充
FetchContent_MakeAvailable(googletest)

## CMake Cookbook中关于添加gtest还添加了下面一行
## 用于查询内容是否已经被获取或填充
#FetchContent_GetProperties(googletest)

## 如果还没有填充，获取并配置子项目
#if(NOT googletest_POPULATED)
#    FetchContent_Populate(googletest)

## 也就是目前给出了两种教程完成同一个任务，暂时来看，CMake Cookbook给出的似乎更好，
## 实际上需要了解FetchContent_MakeAvailable中的细节再去做判断

enable_testing()

add_executable(hello_test
        vector_signed_area.cpp
        hello_test.cpp
        add_char_calculata.cpp
        test_add_char_calculata.cpp
        the_centre_of_a_circle.cpp
        segment_end_ponit_and_gradient.cpp
        convex_hull.cpp
        bracket.cpp
        cut_rod.cpp
        test_tree_node.cpp
        test_class.cpp
        test_template.cpp
        test_plane_sweep.cpp
        ear_clip.cpp
        test_ear_clip.cpp
        test_mutex.cpp
        test_intersect.cpp
        test_signed_area.cpp
        test_cut_rod.cpp)



include_directories(/opt/homebrew/include/glm)
include(GoogleTest)
gtest_discover_tests(hello_test)
target_link_libraries(hello_test GTest::gtest_main)

add_executable(test_mutex
#        test_curve.cpp
        test_thread_fibonacci.cpp
        test_dot_product.cpp
        test_matrix_view_model_projection.cpp
        test_mutex.cpp)
gtest_discover_tests(test_mutex)
target_link_libraries(test_mutex GTest::gtest_main)

add_executable(fixture sample3.cpp include/sample3.h)
gtest_discover_tests(fixture)
target_link_libraries(fixture GTest::gtest_main)

#find_package(Eigen3 REQUIRED)
#INCLUDE_DIRECTORIES ( "$ENV{EIGEN3_INCLUDE_DIR}" )
include_directories(include)
#include_directories()
include_directories(/opt/homebrew/Cellar/eigen/3.4.0_1/include/eigen3)
add_executable(hello_mac
        include/run_time.cpp
        main.cpp
        make_heap.cpp
        thread.cpp
        cut_rod.cpp
        run_priority_queue.cpp
        run_queue.cpp
        longest_continuous_sequence.cpp
        set_graph.cpp)
target_link_libraries(hello_mac ${CURSES_LIBRARIES})