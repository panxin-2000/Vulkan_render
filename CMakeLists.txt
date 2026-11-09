cmake_minimum_required(VERSION 3.27)
project(vulkan_render)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # 关闭 GNU 扩展，确保标准严格对齐

# 强制要求 Abseil 使用完全相同的标准
set(ABSL_PROPAGATE_CXX_STD ON CACHE BOOL "" FORCE)

# 使用现代化 CMake 语法来全局定义 VK_PROTOTYPES 宏，绝不污染 CXX_FLAGS
add_compile_definitions(VK_PROTOTYPES)

#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)


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


include_directories(${CURSES_INCLUDE_DIR})
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -save-temps=obj")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -save-temps=obj")

add_compile_options(-O0)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-fno-strict-return)
endif ()


add_compile_options(-fno-omit-frame-pointer)
add_compile_options(-mno-omit-leaf-frame-pointer)


# tinygltf 中占据了 IMGUI_DIR 这个符号，所需需要放在 tinygltf 之前
set(IMGUI_DIR ./imgui)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends)

file(GLOB imgui_sources
        ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
        ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp)


#add_subdirectory(external/cereal)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(external/googletest)
add_subdirectory(external/quill)


set(MSDFGEN_USE_VCPKG OFF)
set(MSDFGEN_USE_SKIA OFF)


add_subdirectory(simdjson)
add_subdirectory(fastgltf)
add_subdirectory(external/msdfgen)
add_subdirectory(external/meshoptimizer)


# 【核心步骤】通过 CMake 选项关闭整个 KTX 仓库的其他多余目标
set(KTX_FEATURE_TOOLS OFF CACHE BOOL "" FORCE) # 关闭命令行工具 (ktxsc, toktx 等)
set(KTX_FEATURE_TESTS OFF CACHE BOOL "" FORCE) # 关闭 KTX 自带的单元测试
set(KTX_FEATURE_JNI OFF CACHE BOOL "" FORCE)   # 关闭 Java 绑定
set(KTX_FEATURE_PY OFF CACHE BOOL "" FORCE)    # 关闭 Python 绑定

# 只编译运行库，如果你只需要读取纹理，可以把这一项设为 ON 以启用极致裁剪版
#set(LIBKTX_VERSION_READ_ONLY ON CACHE BOOL "" FORCE)


add_subdirectory(external/ktx)
add_subdirectory(external/cxxopts)
add_subdirectory(external/readerwriterqueue)

add_subdirectory(external/libccd)
add_subdirectory(external/entt)
add_subdirectory(external/absl)

find_package(Eigen3 REQUIRED NO_MODULE)
find_package(spirv_cross_core REQUIRED)
find_package(spirv_cross_glsl REQUIRED)
find_package(spirv_cross_cpp REQUIRED)
find_package(TBB REQUIRED)
#find_package(realsense2 REQUIRED)
if (APPLE)
    list(APPEND CMAKE_MODULE_PATH "/opt/homebrew/lib/cmake/OpenVDB")
elseif (LINUX)
    include_directories("/usr/local/include")
    list(APPEND CMAKE_MODULE_PATH "/usr/lib/aarch64-linux-gnu/cmake/OpenVDB")
endif ()

find_package(OpenVDB REQUIRED)
find_package(harfbuzz REQUIRED)
find_package(TBB REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(Curses REQUIRED)
#find_package(benchmark REQUIRED)
find_package(Freetype REQUIRED)

#add_subdirectory(benchmark)


option(TRACY_ENABLE "Enable Tracy profiling" OFF)


add_subdirectory(tracy)
# 优先级从高到低为：
# 局部变量 set(VAR ...) > 命令行 -DVAR=... > CMakeCache.txt 中的缓存值 > option(VAR ... DEFAULT) 源码默认值


# ==============================================================================
# 1. 查找 Open CASCADE
# ==============================================================================
# 推荐显式指定你需要用到的 OCCT 核心组件（Toolkit）
# TKKernel      - 核心底层 (Standard_Handle, Standard_CString等)
# TKBRep        - 边界表示法核心 (TopoDS_Shape, BRep_Tool)
# TKMath        - 数学库 (gp_Pnt, gp_Dir)
# TKMesh        - 网格化模块 (BRepMesh_IncrementalMesh)
# TKSTEP        - STEP 文件导入 (STEPControl_Reader)
# TKV3d / TKAIS - AIS 可视化核心 (AIS_InteractiveContext, AIS_Shape)


set(WITH_VULKAN_BACKEND ON)
set(WITH_OPENGL_BACKEND OFF)


if (WITH_VULKAN_BACKEND)
    add_definitions(-DWITH_VULKAN_BACKEND)
    #######################################vulkan#####################################
    # Use FindVulkan module added with CMAKE 3.7
    if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
        message(STATUS "Using module to find Vulkan")
        find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)
        find_package(glslang CONFIG REQUIRED)

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
    add_subdirectory(external/VulkanMemoryAllocator)


    include_directories(external/ktx/include)
    include_directories(external/ktx/other_include)

    add_subdirectory(render)


endif ()
# 极致性能优化
#set(QUILL_USE_BOUNDED_QUEUE ON CACHE BOOL "" FORCE)


if (WITH_OPENGL_BACKEND)
    add_definitions(-DWITH_OPENGL_BACKEND)
    link_libraries("-framework OPENGL")

endif ()

include_directories(utility)
include_directories(object_model)


#link_directories(/usr/local/lib/slang/lib)


include_directories(external/tinyobj)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")


file(GLOB_RECURSE object_model
        "object_model/load_gltf_model.cpp"
        "object_model/nano_vdb_model.cpp"
        "object_model/deferred_pass.cpp"
        #        "object_model/manifold_model.cpp"
        "object_model/simple_computer_buffer.cpp"
        "object_model/single_char.cpp"
        "object_model/skybox.cpp"
)


file(GLOB_RECURSE component
        "UI/UI_block.cpp"
        "UI/UI_button.cpp"
        "UI/UI_button_render.cpp"
        "UI/UI_imgui.cpp"
        "UI/UI_text.cpp"
        "UI/UI_text_render.cpp"
        "UI/UI_line_render.cpp"
        "UI/UI_render_common.cpp"
        "UI/font_manager.cpp"
        "UI/bin_pack/Rect.cpp"
        "UI/bin_pack/MaxRectBinPact.cpp"
        "UI/bin_pack/ShelfBinPack.cpp"
        "UI/bin_pack/SkylineBinPack.cpp"
        "UI/bin_pack/GuillotineBinPack.cpp"
        "object_model/base_render_object.cpp"
        "object_model/base_3d_render_object.cpp"
        "UI/base_2d_render_object.cpp"
        "object_model/object_line.cpp"
        "object_model/object_ply.cpp"
        "object_model/PLYLoader.cpp"
        "component/scene_component.cpp"
        "component/camera_optical_component.cpp"
        "component/transform_component.cpp"
        "component/CircularBuffer.cpp"
        "component/ashmem.cpp"
        "component/architecture.cpp"
        "component/farmhash.cpp"
        "component/Path.cpp"
        "component/Rect_2D_component.cpp"
        "component/UI_manager.cpp"
        "component/input_component.cpp"
        "component/world_scene_root.cpp"
)


include_directories(windows)
file(GLOB_RECURSE WINDOW
        " windows/windows.cpp"
)
if (APPLE)
    add_definitions(-DVK_USE_PLATFORM_MACOS_MVK -DVK_USE_PLATFORM_METAL_EXT -DVK_ENABLE_BETA_EXTENSIONS)
endif ()
include_directories(event)
include_directories(singleton)
include_directories(labyrinth)
include_directories(input_device)
include_directories(component)
include_directories(interactable_object)
include_directories(spherical_harmonics)


file(GLOB_RECURSE LABYRINTH
        "labyrinth/labyrinth.cpp"
        "labyrinth/set_graph.cpp"
)
file(GLOB_RECURSE INPUT_DEVICE
        "input_device/key_map_value.cpp"
        "input_device/device_input_event_deal.cpp"
)


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


enable_testing()

add_executable(cache_time
        test/hello_test.cpp
)

add_executable(hello_test ${LABYRINTH}
        #        spherical_harmonics/spherical_harmonics_test.cpp
        test/add_char_calculata.cpp
        test/test_openvdb.cpp
        test/test_add_char_calculata.cpp
        test/the_centre_of_a_circle.cpp
        test/segment_end_ponit_and_gradient.cpp
        test/longest_continuous_sequence.cpp
        test/test_delaunay_triangulation.cpp
        test/test_tetrahedron.cpp
        test/convex_hull.cpp
        test/bracket.cpp
        test/test_QEM.cpp
        test/test_factory.cpp
        test/cut_rod.cpp
        test/test_tree_node.cpp
        test/test_class.cpp
        test/test_SH.cpp
        test/knapsack.cpp
        test/test_template.cpp
        test/test_plane_sweep.cpp
        test/ear_clip.cpp
        test/test_Tent_Filter.cpp
        test/test_ear_clip.cpp
        test/test_mutex.cpp
        test/test_best-fit.cpp
        test/test_occt.cpp
        test/test_quadtree.cpp
        test/test_particle.cpp
        test/test_intersect.cpp
        test/test_reader_writer_queue.cpp
        test/test_lazy_Evaluation.cpp
        test/test_signed_area.cpp
        test/test_conveyor_belt.cpp
        test/test_cut_rod.cpp)


include_directories(/opt/homebrew/include/glm)
include(GoogleTest)
gtest_discover_tests(hello_test DISCOVERY_TIMEOUT 30)
gtest_discover_tests(cache_time)
target_link_libraries(hello_test ccd hwy readerwriterqueue OpenVDB::openvdb GTest::gtest_main TBB::tbb Eigen3::Eigen EnTT::EnTT quill::quill)
target_compile_definitions(hello_test PRIVATE VK_NO_PROTOTYPES)
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
        test/test_thread_fibonacci.cpp
        test/test_dot_product.cpp
        test/test_matrix_view_model_projection.cpp
        test/test_mutex.cpp)
gtest_discover_tests(test_mutex)
target_link_libraries(test_mutex GTest::gtest_main)

add_executable(fixture test/sample3.cpp include/sample3.h)
gtest_discover_tests(fixture)
target_link_libraries(fixture GTest::gtest_main)

add_subdirectory(include)

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


#find_library(TCMALLOC_LIB NAMES tcmalloc_minimal tcmalloc)
#target_link_libraries(my_program PRIVATE ${TCMALLOC_LIB})

# 主要用于解决 Windows 平台上的命名冲突和数学常量支持问题
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")

# 可以更容易地使用相对路径（如 ./shaders/main.spv）来定位资源
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")


add_executable(learn_render main.cpp
        object_model/tiny_obj_loader.cc

        ${component}
        ${RENDER}
        ${object_model}
        ${INPUT_DEVICE}
        ${LABYRINTH}
        ${WINDOW}
        "object_model/parse_geometry_file.cpp"
        "UI/sync_proxy_to_render_thread.cpp"
        "UI/VKR_proxy_component.cpp"
        "UI/opacity_batch.cpp"

        ${imgui_sources}
)

target_link_libraries(learn_render PRIVATE project_tracy)

target_compile_definitions(learn_render PRIVATE VK_NO_PROTOTYPES)


set_target_properties(${NAME} PROPERTIES DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})


target_link_libraries(learn_render PRIVATE TBB::tbb Eigen3::Eigen EnTT::EnTT quill::quill)

target_link_libraries(learn_render PRIVATE SDL3::SDL3)
target_link_libraries(learn_render PRIVATE SDL3::SDL3-shared)
target_link_libraries(learn_render PRIVATE OpenVDB::openvdb)
target_link_libraries(learn_render PRIVATE fastgltf::fastgltf)
target_include_directories(learn_render PRIVATE ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(learn_render PRIVATE ${FREETYPE_LIBRARIES})

target_link_libraries(learn_render PRIVATE harfbuzz::harfbuzz)
target_link_libraries(learn_render PRIVATE msdfgen-core)
target_link_libraries(learn_render PRIVATE msdfgen-ext)
target_link_libraries(learn_render PRIVATE simdjson::simdjson)
target_link_libraries(learn_render PRIVATE Vulkan::shaderc_combined)
target_link_libraries(learn_render PRIVATE glslang::glslang)
target_link_libraries(learn_render PRIVATE glslang::SPIRV)
target_link_libraries(learn_render PRIVATE spirv-cross-core)
target_link_libraries(learn_render PRIVATE spirv-cross-glsl)
target_link_libraries(learn_render PRIVATE spirv-cross-cpp)

#target_link_libraries(learn_render PRIVATE TBB::tbbmalloc_proxy)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(learn_render PRIVATE -fsanitize=undefined)
    target_link_options(learn_render PRIVATE -fsanitize=undefined)
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # 将部分严格的编译错误弱化为警告，强行让代码通过编译
    add_compile_options(-fpermissive)
endif ()


# 【只选择其一】在这里只链接你需要的轻量库
# 如果你上面开启了 LIBKTX_VERSION_READ_ONLY，请链接：ktx_read
# 如果你需要完整的写功能，请链接：ktx
target_link_libraries(learn_render PRIVATE project_include)
target_link_libraries(learn_render PRIVATE ktx)
target_link_libraries(learn_render PRIVATE cxxopts::cxxopts)
target_link_libraries(learn_render PRIVATE readerwriterqueue)
target_link_libraries(learn_render PRIVATE meshoptimizer)
target_link_libraries(learn_render PRIVATE ccd)
target_link_libraries(learn_render PRIVATE render_vulkan)


if (APPLE)
    # 1. 自动在构建目录生成凭证文件
    set(ENTITLEMENTS_FILE "${CMAKE_CURRENT_BINARY_DIR}/debug.entitlements")
    file(WRITE ${ENTITLEMENTS_FILE}
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://apple.com\">\n"
            "<plist version=\"1.0\">\n"
            "<dict>\n"
            "    <!-- 允许 Instruments 挂载调试 -->\n"
            "    <key>com.apple.security.get-task-allow</key>\n"
            "    <true/>\n"
            "    <!-- 允许加载未签名的第三方动态库（如 Instruments 的 liboainject.dylib） -->\n"
            "    <key>com.apple.security.cs.disable-library-validation</key>\n"
            "    <true/>\n"
            "    <!-- 允许执行或注入处于不可执行内存段的代码 -->\n"
            "    <key>com.apple.security.cs.allow-unsigned-executable-memory</key>\n"
            "    <true/>\n"
            "</dict>\n"
            "</plist>\n"
    )

    # 2. 让目标程序在编译完成后自动签名（假设你的 target 叫 MyVulkanApp）
    add_custom_command(TARGET learn_render POST_BUILD
            COMMAND codesign --force --sign - --entitlements ${ENTITLEMENTS_FILE} $<TARGET_FILE:learn_render>
            COMMENT "Automatically signing with get-task-allow for Instruments profiling..."
    )
endif ()
