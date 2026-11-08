//
// Created by 潘鑫 on 2026/3/18.
//
#include "UI/UI_button.h"

#include "input_component.h"
#include "base_event.h"
#include "render_component/Geometry_component.h"
#include "model_matrix.h"
#include "../include/name_component.h"
#include "Rect_2D_component.h"
#include "render_state.h"
#include "UI/UI_text.h"


#include  "bounding_box_2d.h"//
#include "engine.h"


void update_UI_button_render_para(const entt::entity entity) {
    int logical_w, logical_h;
    const auto &backend = VK_backend::instance();
    SDL_GetWindowSize(backend.get_window(), &logical_w, &logical_h);
    Eigen::Vector2f scale{2.0f / logical_w, 2.0f / logical_h};
    Eigen::Vector2f translate{-1.0f - 0 * scale[0], -1.0f - 0 * scale[1]};
    set_push_constant_parameter(entity, "uScale", scale);
    set_push_constant_parameter(entity, "uTranslate", translate);
}


void set_round_box_impl(const entt::entity entity) {
    auto proxy_entity = get_proxy_entity(entity);
    logic_update_proxy<Name_component>(entity);

    Logic_entt().remove<Geometry_data>(entity); // 这里是很重要的一项，需要清除之前的几何数据

    auto box = Logic_entt().get<Bounding_Box_2D>(entity);
    add_2D_bound_box_geometry(entity, {box.min_x(), box.min_y()}, {box.max_x(), box.max_y()});
    auto RoundRect = Logic_entt().get<UI_Button_data>(entity);

    auto shader_path = VKR_shader_paths{
        "2D/vulkan_round_box", "2D/vulkan_round_box", "", ""
    };
    if (!Logic_entt().all_of<VKR_shader_paths>(entity)) {
        Logic_entt().emplace_or_replace<VKR_shader_paths>(entity, shader_path);
        logic_update_proxy<VKR_shader_paths>(entity);
    }
    if (!Logic_entt().all_of<Shader_data>(entity)) {
        auto shader_data = Engine::instance().get_shader_manager().find(shader_path);
        Logic_entt().emplace_or_replace<Shader_data>(entity, shader_data);
        logic_update_proxy<Shader_data>(entity);
    }


    logic_set_render_parameter(entity, "round_box", RoundRect);


    update_UI_button_render_para(entity);
    logic_update_proxy(entity, get_VKR_mesh(entity));
    auto primitives = create_primitives(entity);
    logic_update_proxy(entity, primitives);
    std::vector<VKR_Render_state> render_states;
    VKR_Render_state render_state;
    render_state.set_front_face(VK_FRONT_FACE_COUNTER_CLOCKWISE);
    render_state.set_VkCullModeFlags(VK_CULL_MODE_NONE);
    render_state.set_depthTest_disable();
    render_states.push_back(render_state);
    logic_update_proxy(entity, render_states);
}


void draw_UI_button(const entt::entity entity) {
    if (Logic_entt().all_of<Tag_UI_Button_data_dirty, Tag_UI_Button_renderable>(entity)) {
        set_round_box_impl(entity);
        logic_update_add_tag<Tag_UI_Button>(entity);
        Logic_entt().remove<Tag_UI_Button_data_dirty>(entity);
    }
}


// logic_render_object &logic_render_object::add_shader_path(VKR_shader_paths shader_path) {
//     Logic_entt().emplace_or_replace<VKR_shader_paths>(entity, shader_path);
//     auto shader_data = Engine::instance().get_shader_manager().find(shader_path);
//     Logic_entt().emplace_or_replace<Shader_data>(entity, shader_data);
//     logic_update_proxy<VKR_shader_paths>(entity);
//     logic_update_proxy<Shader_data>(entity);
//     return *this;
// }
//
// bool logic_render_object::set_random_triangle_color() {
//     if (auto shader_path = Logic_entt().try_get<VKR_shader_paths>(entity)) {
//         shader_path->clear_define_macro();
//         shader_path->add_define_macro("PASS_RANDOM_TRIANGLE_COLOR", 1);
//         auto shader_data = Engine::instance().get_shader_manager().find(*shader_path);
//         Logic_entt().emplace_or_replace<Shader_data>(entity, shader_data);
//         logic_update_proxy<VKR_shader_paths>(entity);
//         logic_update_proxy<Shader_data>(entity);
//         return true;
//     }
//     return false;
// }


// UI_Button::UI_Button(const std::string &debug_name)
//     : object_2d(debug_name) {
//     add_shader_path();
//
//     logic_update_add_tag<UI_2D_tag>(entity);
// }
