//
// Created by 潘鑫 on 2026/3/18.
//
#include "UI/UI_button.h"

#include "input_component.h"
#include "base_event.h"
#include "render_component/Geometry_component.h"
#include "../include/name_component.h"
#include "Rect_2D_component.h"
#include "render_state.h"


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


void draw_UI_button_impl(const entt::entity entity) {
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


bool draw_UI_button(const entt::entity entity) {
    if (Logic_entt().all_of<Tag_UI_Button_data_dirty, Tag_UI_Button_renderable>(entity)) {
        draw_UI_button_impl(entity);
        logic_update_add_tag<Tag_UI_Button>(entity);
        Logic_entt().remove<Tag_UI_Button_data_dirty>(entity);
        return true;
    }
    return false;
}

bool update_UI_button(const entt::entity entity) {
    if (Logic_entt().all_of<Tag_UI_Button>(entity)) {
        Logic_entt().emplace_or_replace<Tag_UI_Button_data_dirty>(entity);
        return true;
    }
    return false;
}
