cmake_minimum_required(VERSION 3.16)
project(project_include LANGUAGES CXX)

add_library(project_include INTERFACE)


target_include_directories(project_include INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/component
        ${CMAKE_CURRENT_SOURCE_DIR}/tree
)
