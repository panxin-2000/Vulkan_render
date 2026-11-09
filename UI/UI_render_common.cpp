//
// Created by 潘鑫 on 2026/11/9.
//

#include "UI_render_common.h"

#include "engine.h"
#include "scene_component.h"
#include "UI_manager.h"
#include "VKR_proxy_component.h"


void add_shader(const entt::entity entity, const VKR_shader_paths &shader_path) {
    if (!Logic_entt().all_of<VKR_shader_paths>(entity)) {
        Logic_entt().emplace_or_replace<VKR_shader_paths>(entity, shader_path);
        logic_update_proxy<VKR_shader_paths>(entity);
    }
    if (!Logic_entt().all_of<Shader_data>(entity)) {
        auto shader_data = Engine::instance().get_shader_manager().find(shader_path);
        Logic_entt().emplace_or_replace<Shader_data>(entity, shader_data);
        logic_update_proxy<Shader_data>(entity);
    }
}


/**
 * 这里的函数是稍微有点问题的，没新增加一个类别，都需要添加
 * @param itself_entity
 */
void set_render_2D_data_dirty(const entt::entity itself_entity) {
    auto lambda = [](const entt::entity entity_temp) {
        update_UI_button(entity_temp);
        update_UI_text(entity_temp);
        update_UI_line(entity_temp);
    };
    add_recursion_function_to_itself_children(itself_entity, lambda);
}


void redraw_render_2D_data(float time_milliseconds) {
    auto UI_root = get_UI_scene_root();
    auto lambda  = [](const entt::entity entity_temp)-> bool {
        if (draw_UI_button(entity_temp)) return true;
        if (draw_UI_text(entity_temp)) return true;
        if (draw_UI_line(entity_temp)) return true;
        return false;
    };
    add_recursion_function_to_children_condition(UI_root, lambda);
}
