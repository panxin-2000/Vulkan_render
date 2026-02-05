cmake_minimum_required(VERSION 3.27)
project(hello_mac)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVK_PROTOTYPES")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_PROTOTYPES")


#IF (WIN32)
#    # Nothing here (yet)
#ELSEIF (APPLE)
#    # 如果使用的 glfw 那么是不用的
#    link_libraries(${Vulkan_LIBRARY} "-framework AppKit" "-framework QuartzCore")
#    # 这一行不知道有什么作用，但是 target_link_libraries 我还是知道有什么作用的
#    # -framework AppKit: 必须链接。用于处理窗口管理和用户界面（如 NSView），这是创建 Vulkan Surface 的基础
#    # -framework QuartzCore: 必须链接。它提供了 CAMetalLayer，因为在 macOS 上，Vulkan 是通过 MoltenVK 转换到 Metal 运行的，而 Metal 依赖于该框架。
#ELSE (WIN32)
#    link_libraries(${XCB_LIBRARIES} ${Vulkan_LIBRARY} ${Vulkan_LIBRARY} ${DIRECTFB_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
#ENDIF (WIN32)


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
        GIT_TAG v1.17.0
        GIT_SHALLOW TRUE    # 只下载该标签对应的代码，不下载历史
)

#  开发者常私下将其解读为 Entity Templates
FetchContent_Declare(
        entt
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG v3.16.0
        GIT_SHALLOW TRUE    # 只下载该标签对应的代码，不下载历史
)

FetchContent_Declare(
        quill
        GIT_REPOSITORY https://github.com/odygrd/quill.git
        GIT_TAG v11.0.2  # 明确指向某个标签
        GIT_SHALLOW TRUE    # 只下载该标签对应的代码，不下载历史
)
FetchContent_MakeAvailable(quill)


set(WITH_VULKAN_BACKEND ON)
set(WITH_OPENGL_BACKEND OFF)


if (WITH_VULKAN_BACKEND)
    add_definitions(-DWITH_VULKAN_BACKEND)
    #######################################vulkan#####################################
    # Use FindVulkan module added with CMAKE 3.7
    if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
        message(STATUS "Using module to find Vulkan")
        find_package(Vulkan REQUIRED)
    endif ()

    # 关于vulkan的内容，尽量聚拢到一起
    IF (NOT Vulkan_FOUND)
        message(FATAL_ERROR "Could not find Vulkan library!")
    ELSE ()

        message(STATUS "Vulkan include direction" : ${Vulkan_INCLUDE_DIR})
        message(STATUS "Vulkan LIBRARY direction" : ${Vulkan_LIBRARY})
        string(REGEX REPLACE "libvulkan.dylib" "" VULKAN_LIBRARIES_DIRECTORY ${Vulkan_LIBRARY})
        message(STATUS "Vulkan include_directories" : ${VULKAN_LIBRARIES_DIRECTORY})
        include_directories(${Vulkan_INCLUDE_DIR})
        link_directories(${VULKAN_LIBRARIES_DIRECTORY})  # 一个稍微好一点的方式去除掉了最后的文件的内容，而不是用绝对的字符串
        #    我个人感觉find之后引用相应的文件夹比较好，所以我改到这里的了，至于之后链接，这个没有办法了，需要看相应的应用层面的事情了
        # Force validation at compile time
        if (FORCE_VALIDATION)
            add_definitions(-DFORCE_VALIDATION)
        endif ()
    ENDIF ()

    #######################################vulkan#####################################


    add_definitions(-DENGINE_USE_VOLK)
    FetchContent_Declare(
            VulkanMemoryAllocator
            GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
            GIT_TAG v3.1.0  # 建议指定一个稳定的发布版本
            GIT_SHALLOW TRUE    # 只下载该标签对应的代码，不下载历史
            SYSTEM
    )
    FetchContent_MakeAvailable(VulkanMemoryAllocator)


    include_directories(external/ktx/include)
    include_directories(external/ktx/other_include)


    set(KTX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/ktx)
    set(KTX_SOURCES
            ${KTX_DIR}/lib/texture.c
            ${KTX_DIR}/lib/hashlist.c
            ${KTX_DIR}/lib/checkheader.c
            ${KTX_DIR}/lib/swap.c
            ${KTX_DIR}/lib/memstream.c
            ${KTX_DIR}/lib/filestream.c
            ${KTX_DIR}/lib/vkloader.c)
    add_library(ktx STATIC ${KTX_SOURCES})


    file(GLOB_RECURSE vulkan_code
            #        "vulkan_code/glfw_vulkan.cpp"
            "vulkan_code/vulkan_utility.cpp"
            "vulkan_code/vulkan_read_attribute.cpp"
            "vulkan_code/vulkan_validation_layer.cpp"
            "vulkan_code/vulkan_image.cpp"
            "vulkan_code/vulkan_buffer.cpp"
            "vulkan_code/vulkan_device_handle.cpp"
            "vulkan_code/transfer_texture_to_gpu.cpp"
            "vulkan_render/backend.cpp"
    )
    include_directories(vulkan_code)

endif ()
# 极致性能优化
#set(QUILL_USE_BOUNDED_QUEUE ON CACHE BOOL "" FORCE)


if (WITH_OPENGL_BACKEND)
    add_definitions(-DWITH_OPENGL_BACKEND)
    link_libraries("-framework OPENGL")

endif ()

include_directories(utility)
include_directories(render)
include_directories(/opt/homebrew/opt/glfw/include)

link_directories(/opt/homebrew/opt/glfw/lib)
link_directories(/opt/homebrew/opt/glew/lib)

link_directories(/usr/local/lib/slang/lib)


include_directories(external/tinyobj)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
find_package(OpenGL REQUIRED)

find_package(Eigen3 3.4 REQUIRED NO_MODULE)

find_package(spirv_cross_core REQUIRED)
find_package(spirv_cross_glsl REQUIRED)
find_package(spirv_cross_cpp REQUIRED)

find_package(GLEW COMPONENTS
        GLEW::glew
        REQUIRED)
if (GLEW_FOUND)
    add_definitions(-DGLEW_FOUND)
    include_directories(${GLEW_INCLUDE_DIRS})
endif ()

file(GLOB_RECURSE RENDER

        "render_common/stb_image.cpp"
)


include_directories(render_common)
include_directories(windows)
file(GLOB_RECURSE WINDOW
        "windows/windows.cpp"
)
if (APPLE)
    add_definitions(-DVK_USE_PLATFORM_MACOS_MVK -DVK_USE_PLATFORM_METAL_EXT -DVK_ENABLE_BETA_EXTENSIONS)
endif ()
include_directories(event)
include_directories(singleton)
include_directories(labyrinth)
include_directories(input_device)
include_directories(vulkan_render)
include_directories(component)
include_directories(interactable_object)


file(GLOB_RECURSE LABYRINTH
        "labyrinth/labyrinth.cpp"
        "labyrinth/set_graph.cpp"
)
file(GLOB_RECURSE INPUT_DEVICE
        "input_device/key_map_value.cpp"
        "input_device/device_input_event_deal.cpp"
)

find_package(TBB REQUIRED)


#######################################vulkan#####################################

# Use FindVulkan module added with CMAKE 3.7
if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
    message(STATUS "Using module to find Vulkan")
    find_package(Vulkan REQUIRED)
endif ()

## 关于vulkan的内容，尽量聚拢到一起
IF (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library!")
ELSE ()

    message(STATUS "Vulkan include direction" : ${Vulkan_INCLUDE_DIR})
    message(STATUS "Vulkan LIBRARY direction" : ${Vulkan_LIBRARY})
    string(REGEX REPLACE "libvulkan.dylib" "" VULKAN_LIBRARIES_DIRECTORY ${Vulkan_LIBRARY})
    message(STATUS "Vulkan include_directories" : ${VULKAN_LIBRARIES_DIRECTORY})
    include_directories(${Vulkan_INCLUDE_DIR})
    link_directories(${VULKAN_LIBRARIES_DIRECTORY})  # 一个稍微好一点的方式去除掉了最后的文件的内容，而不是用绝对的字符串
    #    我个人感觉find之后引用相应的文件夹比较好，所以我改到这里的了，至于之后链接，这个没有办法了，需要看相应的应用层面的事情了
    # Force validation at compile time
    if (FORCE_VALIDATION)
        add_definitions(-DFORCE_VALIDATION)
    endif ()
ENDIF ()


set(LIBRARIES "Vulkan::Vulkan")

#######################################vulkan#####################################


## 下面这行没有看懂什么意思，按照说明先复制过来了
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
#FetchContent_MakeAvailable() 命令会确保指定的依赖项已被填充
FetchContent_MakeAvailable(googletest)
FetchContent_MakeAvailable(entt)

## CMake Cookbook中关于添加gtest还添加了下面一行
## 用于查询内容是否已经被获取或填充
#FetchContent_GetProperties(googletest)

## 如果还没有填充，获取并配置子项目
#if(NOT googletest_POPULATED)
#    FetchContent_Populate(googletest)

## 也就是目前给出了两种教程完成同一个任务，暂时来看，CMake Cookbook给出的似乎更好，
## 实际上需要了解FetchContent_MakeAvailable中的细节再去做判断

enable_testing()

add_executable(hello_test ${RENDER} ${WINDOW} ${INPUT_DEVICE} ${LABYRINTH}
        test/hello_test.cpp
        test/add_char_calculata.cpp
        test/test_add_char_calculata.cpp
        test/the_centre_of_a_circle.cpp
        test/segment_end_ponit_and_gradient.cpp
        test/longest_continuous_sequence.cpp
        test/test_delaunay_triangulation.cpp
        test/test_tetrahedron.cpp
        test/convex_hull.cpp
        test/bracket.cpp
        test/test_factory.cpp
        test/cut_rod.cpp
        test/test_tree_node.cpp
        test/test_class.cpp
        test/test_template.cpp
        test/test_plane_sweep.cpp
        test/ear_clip.cpp
        test/test_ear_clip.cpp
        test/test_mutex.cpp
        test/test_intersect.cpp
        test/test_signed_area.cpp
        test/test_cut_rod.cpp)


include_directories(/opt/homebrew/include/glm)
include(GoogleTest)
gtest_discover_tests(hello_test)
target_link_libraries(hello_test GTest::gtest_main GLFW TBB::tbb GLEW Eigen3::Eigen EnTT::EnTT)

add_executable(test_mutex
        #        test_curve.cpp
        test/test_thread_fibonacci.cpp
        test/test_dot_product.cpp
        test/test_matrix_view_model_projection.cpp
        test/test_mutex.cpp)
gtest_discover_tests(test_mutex)
target_link_libraries(test_mutex GTest::gtest_main)

add_executable(fixture test/sample3.cpp include/sample3.h)
gtest_discover_tests(fixture)
target_link_libraries(fixture GTest::gtest_main)

#find_package(Eigen3 REQUIRED)
#INCLUDE_DIRECTORIES ( "$ENV{EIGEN3_INCLUDE_DIR}" )
include_directories(include)
#include_directories()
include_directories(/opt/homebrew/Cellar/eigen/3.4.0_1/include/eigen3)
add_executable(hello_mac
        include/run_time.cpp
        test/make_heap.cpp
        test/thread.cpp
        test/cut_rod.cpp
        test/run_priority_queue.cpp
        test/run_queue.cpp
        test/longest_continuous_sequence.cpp
        test/set_graph.cpp)
target_link_libraries(hello_mac ${CURSES_LIBRARIES} EnTT::EnTT quill::quill)


find_package(TBB REQUIRED)

# 主要用于解决 Windows 平台上的命名冲突和数学常量支持问题
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")

# 可以更容易地使用相对路径（如 ./shaders/main.spv）来定位资源
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")


add_executable(learn_render main.cpp external/tinyobj/tiny_obj_loader.cc assets/shader.slang
        ${RENDER} ${INPUT_DEVICE} ${LABYRINTH} ${WINDOW} ${vulkan_code})


target_compile_definitions(learn_render PRIVATE VK_NO_PROTOTYPES)


set_target_properties(${NAME} PROPERTIES DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})


target_link_libraries(learn_render PRIVATE TBB::tbb GLEW Eigen3::Eigen EnTT::EnTT quill::quill)

target_link_libraries(learn_render PRIVATE ktx slang)
target_link_libraries(learn_render PRIVATE volk)
target_link_libraries(learn_render PRIVATE GLFW)
target_link_libraries(learn_render PRIVATE spirv-cross-cpp)

target_link_libraries(learn_render PRIVATE GPUOpen::VulkanMemoryAllocator)

add_custom_command(
        TARGET learn_render
        PRE_BUILD
        COMMAND glslc ${CMAKE_CURRENT_SOURCE_DIR}/render/shader/temp.vert -o
        ${CMAKE_CURRENT_SOURCE_DIR}/render/shader/temp.vert.spv
)
add_custom_command(
        TARGET learn_render
        PRE_BUILD
        COMMAND glslc ${CMAKE_CURRENT_SOURCE_DIR}/render/shader/temp.frag -o
        ${CMAKE_CURRENT_SOURCE_DIR}/render/shader/temp.frag.spv
)
add_custom_command(
        TARGET learn_render
        PRE_BUILD
        COMMAND glslc ${CMAKE_CURRENT_SOURCE_DIR}/render/shader/vulkan_different_color.vert -o
        ${CMAKE_CURRENT_SOURCE_DIR}/render/shader/vulkan_different_color.vert.spv
)
add_custom_command(
        TARGET learn_render
        PRE_BUILD
        COMMAND glslc ${CMAKE_CURRENT_SOURCE_DIR}/render/shader/vulkan_different_color.frag -o
        ${CMAKE_CURRENT_SOURCE_DIR}/render/shader/vulkan_different_color.frag.spv
)


# 自动执行命令编译 glsl 的脚本
#cmake_minimum_required(VERSION 3.19)
#project(Test)
#
#set( ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
#
#set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
#file(GLOB SHADERS ${SHADER_DIR}/*.vert ${SHADER_DIR}/*.frag ${SHADER_DIR}/*.comp ${SHADER_DIR}/*.geom ${SHADER_DIR}/*.tesc ${SHADER_DIR}/*.tese ${SHADER_DIR}/*.mesh ${SHADER_DIR}/*.task ${SHADER_DIR}/*.rgen ${SHADER_DIR}/*.rchit ${SHADER_DIR}/*.rmiss)
#
#file(GLOB SPV_SHADERS ${SHADER_DIR}/*.spv)
#
#find_package(Vulkan)
#
#foreach(SHADER IN LISTS SHADERS)
#get_filename_component(FILENAME ${SHADER} NAME)
#add_custom_command(OUTPUT ${SHADER_DIR}/shaders/${FILENAME}.spv
#COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${SHADER_DIR}/shaders/${FILENAME}.spv
#DEPENDS ${SHADER}
#COMMENT "Compiling ${FILENAME}")
#endForeach()
#
#add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
#
#add_executable(Test main.cpp ${SHADERS})
#add_dependencies(Test shaders)
