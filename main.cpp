/* Copyright (c) 2025-2026, Sascha Willems
 * SPDX-License-Identifier: MIT
 */

#include <GLFW/glfw3.h>

#include <thread>
#include "vulkan_render/backend.h"
#include "vulkan_render/vulkan_render_manage.h"
#include "vulkan_device_handle.h"
#include "event/base_event.h"
#include "labyrinth.h"
#include "UI/UI_block.h"
#include "UI/UI_button.h"

#include "global_singleton.h"

void register_glfw(GLFWwindow *window);

void deal_glfw_event();


#include <iostream>
#include <fstream>
#include <string>
#include <regex>


#include <string>
#include <unordered_map>
#include <vulkan/vulkan.h>

/**
 * Maps GLSL keywords extracted by regex to Vulkan Descriptor Types.
 * This is essential for building VkDescriptorSetLayoutBinding.
 */
VkDescriptorType get_descriptor_type(const std::string &keyword, const std::string &full_match) {
    // 1. Define the mapping table
    static const std::unordered_map<std::string, VkDescriptorType> type_map = {
        {"uniform", VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER},
        {"buffer", VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
        {"sampler2D", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER},
        {"sampler3D", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER},
        {"samplerCube", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER},
        {"image2D", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE},
        {"image3D", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE},
        {"samplerBuffer", VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER},
        {"imageBuffer", VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER},
        {"subpassInput", VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT}
    };

    // 2. Special Logic: Detect if 'uniform' refers to a Buffer or an Image
    // In GLSL: 'uniform sampler2D' is a sampler, but 'uniform MyBlock {}' is a buffer.
    if (keyword == "uniform") {
        // If the full line contains common sampler types, it's a sampler
        if (full_match.find("sampler") != std::string::npos) {
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        }
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }

    // 3. Standard lookup
    auto it = type_map.find(keyword);
    if (it != type_map.end()) {
        return it->second;
    }

    // Default fallback
    return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}


void parseBindings(const std::string &filePath) {
    std::ifstream file(filePath);
    std::string line;
    // std::regex bindingRegex(R"(binding\s*=\s*(\d+))");
    // std::regex bindingRegex(R"(layout\s*\(.*binding\s*=\s*(\d+).*\))");
    std::regex bindingRegex(R"(layout\s*\(.*binding\s*=\s*(\d+).*\)\s*(\w+))");
    // std::regex bindingRegex(R"(layout\s*\(.*binding\s*=\s*(\d+).*\)\s*([^;{]+))");

    while (std::getline(file, line)) {
        std::smatch match;
        if (std::regex_search(line, match, bindingRegex)) {
            std::string binding_id   = match[1].str(); // 第一个括号的内容
            std::string type_keyword = match[2].str(); // 第二个括号的内容
            std::string full_line    = match[0].str();

            std::cout << "Found Binding ID: " << binding_id << " | 类型: " << type_keyword << " in line: " << line <<
                    std::endl;

            VkDescriptorType vk_type = get_descriptor_type(type_keyword, full_line);

            if (vk_type != VK_DESCRIPTOR_TYPE_MAX_ENUM) {
                printf("Binding %s: Assigned to VkDescriptorType %d\n", binding_id.c_str(), vk_type);

                // Now you can fill your Vulkan struct:
                // VkDescriptorSetLayoutBinding b = {};
                // b.binding = std::stoi(binding_id);
                // b.descriptorType = vk_type;
                // b.descriptorCount = 1;
                // b.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT; // or as needed
            }
        }
    }
}


int main(int argc, char *argv[]) {
    parseBindings("/Users/panxin/CLionProjects/Vulkan/shaders/glsl/computenbody/particle_calculate.comp");

    return 0;
    LOG_INFO(g_log(), "Hello from {}!", "Quill v11.0.2");
    auto handle = VKDevice::get();

    render_thread_start(handle);


    // auto entity = get_entt_instance().create();
    // get_entt_instance().emplace<Labyrinth>(entity, "迷宫", entity);
    register_glfw(handle.window_);

    auto block_entity = UI_block("功能块", -0.5, -0.5, 0.5, 0.5);
    // add_button(block_entity, "按钮1", 420, 420, 480, 480);
    // add_button(block_entity, "按钮2", 35, 20, 145, 130);

    auto render        = new logic_render_data;
    render->debug_name = "blender Suzanne";
    render->mesh_path_ = "assets/suzanne.obj";
    render->set_vertex_shader("/Users/panxin/CLionProjects/hello_mac/render/shader/temp.vert.spv");
    render->set_fragment_shader("/Users/panxin/CLionProjects/hello_mac/render/shader/temp.frag.spv");

    add_object_to_render(render); // 因为这里没有区分。全部都在场景的根节点之下

    // Render loop
    while (!glfwWindowShouldClose(handle.window_)) {
        glfwWaitEvents();
        if (GLFW_TRUE == glfwWindowShouldClose(handle.window_)) {
            break;
        }
        glfwPollEvents();  // Event polling
        deal_glfw_event(); // 统一分发执行
        {
            auto view = g_entt().view<Destroy_tag>();   //得到哪些需要销毁
            g_entt().destroy(view.begin(), view.end()); // 执行销毁程序
        } {
            auto view = g_entt().view<Position_update_tag>(); //得到哪些需要销毁
            // g_entt().destroy(view.begin(), view.end());       // todo : 添加新的函数
        }


        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    render_thread_stop_and_wait();

    handle.destroy();
}
