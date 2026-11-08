//
// Created by 潘鑫 on 2026/4/9.
//
#include "UI/UI_text.h"


#include "utf8.h"
#include "json.hpp"  // 仅在需要处理具体业务的 .cpp 文件中引入完整实现
#include <fstream>

#include "UI/base_2d_render_object.h"
#include "bounding_box_2d.h"
#include "engine.h"
#include "font_manager.h"
#include "name_component.h"
#include "render_component/Geometry_component.h"
#include "render_state.h"
#include "scene_component.h"
#include "UI/UI_button.h"

void update_UI_button_render_para(const entt::entity entity);

void draw_UI_text_impl(const entt::entity entity) {
    auto proxy_entity = get_proxy_entity(entity);
    logic_update_proxy<Name_component>(entity);

    Logic_entt().remove<Geometry_data>(entity); // 这里是很重要的一项，需要清除之前的几何数据

    auto &data = Logic_entt().get<UI_Text_data>(entity);


    auto shader_path = VKR_shader_paths{
        "2D/vulkan_MSDF_text", "2D/vulkan_MSDF_text", "", ""
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

    update_UI_button_render_para(entity);


    float min_x = 0;
    float min_y = 0;
    float max_x = 0;
    float max_y = 0;
    if (auto parent = get_parent(entity); parent != entt::null) {
        if (Logic_entt().all_of<Bounding_Box_2D>(parent)) {
            const auto &ui_button_data   = Logic_entt().get<UI_Button_data>(parent);
            auto can_render_bounding_box = ui_button_data.get_render_bounding_box();
            min_x                        = can_render_bounding_box[0];
            min_y                        = can_render_bounding_box[1];
            max_x                        = can_render_bounding_box[2];
            max_y                        = can_render_bounding_box[3];
        }
    }

    auto result = Font_Manager::instance().create_text_geometry_data(data.render_text_,
                                                                     data.font_file_name_,
                                                                     min_x,
                                                                     min_y,
                                                                     max_x,
                                                                     max_y,
                                                                     data.char_size_);
    // 如果可以，尽量考虑圆角部分的内容
    auto scale = SDL_GetWindowDisplayScale(VK_backend::instance().get_window());
    std::vector<VKR_Render_state> render_states;
    VKR_Render_state render_state;
    render_state.scissor.offset.x      = static_cast<int32_t>(min_x * scale);
    render_state.scissor.offset.y      = static_cast<int32_t>(min_y * scale);
    render_state.scissor.extent.width  = static_cast<uint32_t>((max_x - min_x) * scale);
    render_state.scissor.extent.height = static_cast<uint32_t>((max_y - min_y) * scale);
    render_states.push_back(render_state);


    set_geometry_data(entity, result.first, result.second);
    logic_update_proxy(entity, get_VKR_mesh(entity));
    logic_update_proxy(entity, create_primitives(entity));
    logic_update_proxy(entity, render_states);
    logic_update_add_tag<Tag_UI_text>(entity);
}


bool draw_UI_text(const entt::entity entity) {
    if (Logic_entt().all_of<Tag_UI_text_data_dirty, Tag_UI_text_renderable>(entity)) {
        draw_UI_text_impl(entity);
        logic_update_add_tag<Tag_UI_text>(entity);
        Logic_entt().remove<Tag_UI_text_data_dirty>(entity);
        return true;
    }
    return false;
}
