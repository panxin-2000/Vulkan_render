file(GLOB BENCH_FILES "*.cpp")

# 4. 循环为每个文件建立独立的编译目标
foreach (BENCH_SRC ${BENCH_FILES})
    # 从文件名提取出程序名（例如从 bench_octree.cpp 提取出 bench_octree）
    get_filename_component(BENCH_NAME ${BENCH_SRC} NAME_WE)

    # 创建独立的、可执行的测试程序
    add_executable(${BENCH_NAME} ${BENCH_SRC})
    target_compile_options(${BENCH_NAME} PRIVATE -O3 -march=native)
    target_compile_definitions(${BENCH_NAME} PRIVATE NDEBUG)

    include(CheckIPOSupported)
    check_ipo_supported(RESULT lto_supported OUTPUT error)
    if (lto_supported)
        set_target_properties(${BENCH_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif ()
    # 分别链接 Google Benchmark 静态库

    target_link_libraries(${BENCH_NAME} PRIVATE benchmark::benchmark)
endforeach ()
