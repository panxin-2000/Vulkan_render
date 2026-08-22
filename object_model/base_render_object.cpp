//
// Created by 潘鑫 on 2026/8/22.
//


#include "base_render_object.h"

#include "engine.h"
#include "name_component.h"
#include "VKR_proxy_component.h"

VKR_shader_paths get_gltf_shader_path();


logic_render_object::logic_render_object(const std::string &name) : entity(Logic_entt().create()) {
    Logic_entt().emplace<Name_component>(entity, name);
}

logic_render_object &logic_render_object::add_shader_path(VKR_shader_paths shader_path) {
    Logic_entt().emplace_or_replace<VKR_shader_paths>(entity, shader_path);
    auto shader_data = Engine::instance().get_shader_manager().find(shader_path);
    Logic_entt().emplace_or_replace<Shader_data>(entity, shader_data);
    logic_update_proxy<VKR_shader_paths>(entity);
    logic_update_proxy<Shader_data>(entity);
    return *this;
}

bool logic_render_object::set_random_triangle_color() {
    if (auto shader_path = Logic_entt().try_get<VKR_shader_paths>(entity)) {
        shader_path->clear_define_macro();
        shader_path->add_define_macro("PASS_RANDOM_TRIANGLE_COLOR", 1);
        auto shader_data = Engine::instance().get_shader_manager().find(*shader_path);
        Logic_entt().emplace_or_replace<Shader_data>(entity, shader_data);
        logic_update_proxy<VKR_shader_paths>(entity);
        logic_update_proxy<Shader_data>(entity);
        return true;
    }
    return false;
}
