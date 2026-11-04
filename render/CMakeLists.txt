cmake_minimum_required(VERSION 3.16)
project(render_vulkan LANGUAGES CXX)


set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shader)
file(GLOB SHADER_FILES
        "${SHADER_SOURCE_DIR}/*.vert"
        "${SHADER_SOURCE_DIR}/*.frag"
        "${SHADER_SOURCE_DIR}/*.geom"
        "${SHADER_SOURCE_DIR}/*.comp"
)

#add_subdirectory(vulkan_code)

file(GLOB_RECURSE render_code
        "vulkan_code/vulkan_utility.cpp"
        "vulkan_code/vulkan_read_attribute.cpp"
        "vulkan_code/vulkan_validation_layer.cpp"
        "vulkan_code/vulkan_image.cpp"
        "vulkan_code/vulkan_image_view.cpp"
        "vulkan_code/vulkan_image_and_view_discard.cpp"
        "vulkan_code/descriptor_organized_sets_and_bindings.cpp"
        "vulkan_code/vulkan_sample.cpp"
        "vulkan_code/vulkan_buffer.cpp"
        "vulkan_code/vulkan_execute_command.cpp"
        "vulkan_code/vulkan_buffer_block.cpp"
        "vulkan_code/vulkan_backend.cpp"
        "vulkan_code/volk.c"
        "vulkan_code/transfer_texture_to_gpu.cpp"
        "render_thread/backend.cpp"
        "vulkan_code/shader_resolve.cpp"
        "engine.cpp"
        "global_parameters.cpp"
        "VCB_bind_pipeline.cpp"
        "VCB_calculate_frustum_cull.cpp"
        "VCB_compute_command.cpp"
        "VCB_debug_tag.cpp"
        "VCB_direct_render.cpp"
        "VCB_draw_command.cpp"
        "VCB_G_buffer_render.cpp"
        "VCB_shadow_render.cpp"
        "render_image_manager.cpp"
        "Shader_manager.cpp"
        "SSAO_parameters.cpp"
        "VCB_vulkan_command_buffer.cpp"
        "vk_render_to_image.cpp"
        "render_common/stb_image.cpp"
        "vulkan_code/shader_create.cpp"
        "vulkan_code/descriptor.cpp"
        "vulkan_code/create_texture.cpp"
        "vulkan_code/create_pipeline.cpp"
        "vulkan_code/descriptor_pool.cpp"
        "vulkan_code/pipeline_layout.cpp"
        "vulkan_code/sets_and_bindings_layout.cpp"
        "vulkan_code/render_function.cpp"
        "vulkan_code/vulkan_texture_bindless.cpp"
        "vulkan_code/vulkan_update_descriptor.cpp"
        "vulkan_code/updata_push_constants_data.cpp"

        "render_component/Geometry_component.cpp"
        "render_thread/render_thread.cpp"
        "render_thread/VCB_SOA_batch.cpp"
        "render_component/PBR_component.cpp"
        "render_component/pipeline_component.cpp"
        "render_component/pipeline_layout_component.cpp"
        "render_component/shader_component.cpp"
        "render_component/mesh_component.cpp"
        "render_common/Geometry_data.cpp"
)


add_library(render_vulkan STATIC ${render_code})

target_compile_definitions(render_vulkan PRIVATE VK_NO_PROTOTYPES)

target_include_directories(render_vulkan PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../include"     # 先这样写, 之后需要更改
        "${CMAKE_CURRENT_SOURCE_DIR}/../singleton"     # 先这样写, 之后需要更改
        "${CMAKE_CURRENT_SOURCE_DIR}/../tracy/public"     # 先这样写, 之后需要更改
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/render_common
        ${CMAKE_CURRENT_SOURCE_DIR}/render_component
        ${CMAKE_CURRENT_SOURCE_DIR}/vulkan_thread
        ${CMAKE_CURRENT_SOURCE_DIR}/vulkan_code
)

target_link_libraries(render_vulkan PUBLIC Eigen3::Eigen EnTT::EnTT quill::quill)
target_link_libraries(render_vulkan PUBLIC GPUOpen::VulkanMemoryAllocator)
#target_link_libraries(render_vulkan PUBLIC vulkan_core)
target_link_libraries(render_vulkan PUBLIC absl::hash)
target_link_libraries(render_vulkan PUBLIC absl::raw_hash_set)

