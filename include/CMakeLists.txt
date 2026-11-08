cmake_minimum_required(VERSION 3.16)
project(project_include LANGUAGES CXX)

add_library(project_include OBJECT
        run_time.cpp
)


target_include_directories(project_include PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/component
)
