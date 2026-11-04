cmake_minimum_required(VERSION 3.16)
project(simdjson_local LANGUAGES CXX)

# 1. 使用 OBJECT 类型编译这两个单文件，保证轻量和高效
add_library(simdjson_local STATIC
        simdjson.cpp
        simdjson.h
)

set_target_properties(simdjson_local PROPERTIES POSITION_INDEPENDENT_CODE ON)

# 2. 配置头文件搜索路径
# fastgltf 内部通过 #include <simdjson.h> 引用，所以这里必须把当前目录设为 PUBLIC 导出
target_include_directories(simdjson_local PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 3. 创建 fastgltf 梦寐以求的别名目标
add_library(simdjson::simdjson ALIAS simdjson_local)
