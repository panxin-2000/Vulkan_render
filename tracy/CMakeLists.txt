cmake_minimum_required(VERSION 3.16)
project(project_tracy LANGUAGES CXX)


add_library(project_tracy STATIC
        "${CMAKE_CURRENT_SOURCE_DIR}/public/TracyClient.cpp"
)

if (TRACY_ENABLE)
    target_compile_definitions(project_tracy PUBLIC TRACY_ENABLE)
    target_compile_definitions(project_tracy PUBLIC TRACY_MANUAL_LIFETIME)
    target_compile_definitions(project_tracy PUBLIC TRACY_DELAYED_INIT)


    #    add_library(tracy_config INTERFACE)
    #    target_compile_definitions(tracy_config INTERFACE
    #            TRACY_ENABLE
    #            TRACY_MANUAL_LIFETIME
    #            TRACY_DELAYED_INIT
    #    )
    #    target_link_libraries(project_tracy PUBLIC tracy_config)

endif ()


target_include_directories(project_tracy PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/public
)
