
# 1. 🌟 优化：将 LTO/IPO 编译器探测移到循环外部，只执行一次
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT error)

# 2. 自动扫描当前目录下所有的 .cpp 文件
file(GLOB BENCH_FILES "*.cpp")

# 3. 循环为每个文件建立独立的编译目标
foreach (BENCH_SRC ${BENCH_FILES})
    # 从文件名提取出程序名
    get_filename_component(BENCH_NAME ${BENCH_SRC} NAME_WE)

    # 创建独立的、可执行的测试程序
    add_executable(${BENCH_NAME} ${BENCH_SRC})

    # 强制让这个单体程序无论在 Debug 还是 Release 下都拥有最高性能优化和 NDEBUG 宏
    if(NOT MSVC)
        target_compile_options(${BENCH_NAME} PRIVATE -O3 -march=native)
    endif()
    target_compile_definitions(${BENCH_NAME} PRIVATE NDEBUG)

    # 分别链接 Google Benchmark 静态库
    target_link_libraries(${BENCH_NAME} PRIVATE benchmark::benchmark)

    # 4. 🌟 优化：在链接关系确立后，再开启单体目标的链接时优化
    if (lto_supported)
        set_target_properties(${BENCH_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endforeach()
