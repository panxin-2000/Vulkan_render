//
// Created by 潘鑫 on 2026/11/9.
//

#ifndef VULKAN_RENDER_UI_RENDER_COMMON_H
#define VULKAN_RENDER_UI_RENDER_COMMON_H

#include "global_singleton.h"
#include "shader_resolve.h"


void add_shader(const entt::entity entity, const VKR_shader_paths &shader_path);

bool draw_UI_button(const entt::entity entity);

bool draw_UI_text(const entt::entity entity);

bool draw_UI_line(const entt::entity entity);

bool update_UI_button(const entt::entity entity);

bool update_UI_text(const entt::entity entity);

bool update_UI_line(const entt::entity entity);

void set_render_2D_data_dirty(const entt::entity itself_entity);

/**
 * 函数的名义是重新绘制，本质是提交给渲染线程，由渲染线程执行具体的绘制
 * @param time_milliseconds
 */
void redraw_render_2D_data(float time_milliseconds);

#endif //VULKAN_RENDER_UI_RENDER_COMMON_H
