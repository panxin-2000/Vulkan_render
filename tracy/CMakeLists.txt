cmake_minimum_required(VERSION 3.16)
project(project_tracy LANGUAGES CXX)


add_library(project_tracy STATIC
        "${CMAKE_CURRENT_SOURCE_DIR}/public/TracyClient.cpp"
)

if (TRACY_ENABLE)
    target_compile_definitions(project_tracy PUBLIC TRACY_ENABLE)
endif ()


target_include_directories(project_tracy PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/public
)
