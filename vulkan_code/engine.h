//
// Created by 潘鑫 on 2026/1/27.
//

#ifndef HELLO_MAC_ENGINE_H
#define HELLO_MAC_ENGINE_H
#include <list>

#include "vulkan_utility.h"
#include <string>
#include <vector>
#include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>

#include "model_matrix.h"

struct ShaderData {
    matrix_4x4 projection;
    matrix_4x4 view;
    matrix_4x4 model[3];
    float lightPos[4]{0.0f, -10.0f, 10.0f, 0.0f};
    uint32_t selected{1};
    uint32_t selected8{1};
    uint32_t selected7{1};
    uint32_t selected6{1};
};







#endif //HELLO_MAC_ENGINE_H
