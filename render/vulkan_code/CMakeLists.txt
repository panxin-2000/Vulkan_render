cmake_minimum_required(VERSION 3.16)
project(vulkan_core LANGUAGES CXX)


file(GLOB_RECURSE vulkan_code_file
        "vulkan_utility.cpp"
        "vulkan_read_attribute.cpp"
        "vulkan_validation_layer.cpp"
        "vulkan_image.cpp"
        "vulkan_image_view.cpp"
        "vulkan_image_and_view_discard.cpp"
        "descriptor_organized_sets_and_bindings.cpp"
        "vulkan_sample.cpp"
        "vulkan_buffer.cpp"
        "vulkan_execute_command.cpp"
        "vulkan_buffer_block.cpp"
        "vulkan_backend.cpp"
        "volk.c"
        "transfer_texture_to_gpu.cpp"
        "shader_resolve.cpp"
        "shader_create.cpp"
        "descriptor.cpp"
        "create_texture.cpp"
        "create_pipeline.cpp"
        "descriptor_pool.cpp"
        "pipeline_layout.cpp"
        "sets_and_bindings_layout.cpp"
        "render_function.cpp"
        "vulkan_texture_bindless.cpp"
        "vulkan_update_descriptor.cpp"
        "updata_push_constants_data.cpp"
)


add_library(vulkan_core STATIC ${vulkan_code_file})

target_compile_definitions(vulkan_core PRIVATE VK_NO_PROTOTYPES)

target_include_directories(vulkan_core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        "${CMAKE_CURRENT_SOURCE_DIR}/../include"     # 先这样写, 之后需要更改

)

target_link_libraries(vulkan_core PUBLIC Eigen3::Eigen quill::quill)
target_link_libraries(vulkan_core PUBLIC GPUOpen::VulkanMemoryAllocator)
target_link_libraries(vulkan_core PUBLIC absl::hash)
target_link_libraries(vulkan_core PUBLIC absl::raw_hash_set)

