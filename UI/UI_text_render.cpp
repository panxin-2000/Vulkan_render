//
// Created by 潘鑫 on 2026/4/9.
//
#include "UI/UI_text.h"


#include "bounding_box_2d.h"
#include "engine.h"
#include "font_manager.h"
#include "name_component.h"
#include "render_component/Geometry_component.h"
#include "render_state.h"
#include "scene_component.h"
#include "UI_render_common.h"

void update_UI_button_render_para(const entt::entity entity);

void draw_UI_text_impl(const entt::entity entity) {
    auto proxy_entity = get_proxy_entity(entity);
    logic_update_proxy<Name_component>(entity);

    Logic_entt().remove<Geometry_data>(entity); // 这里是很重要的一项，需要清除之前的几何数据

    auto &data = Logic_entt().get<UI_Text_data>(entity);


    auto shader_path = VKR_shader_paths{
        "2D/vulkan_MSDF_text", "2D/vulkan_MSDF_text", "", ""
    };
    add_shader(entity, shader_path);

    update_UI_button_render_para(entity);


    float min_x       = 0;
    float min_y       = 0;
    float max_x       = 0;
    float max_y       = 0;
    auto bounding_box = get_parent_Bounding_Box(entity);
    min_x             = bounding_box.min_x();
    min_y             = bounding_box.min_y();
    max_x             = bounding_box.max_x();
    max_y             = bounding_box.max_y();


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

/**
 * 负责重新绘制，根据当前的参数和它的parent_entity 的参数来完成重新绘制
 * @param entity
 * @return
 */
bool draw_UI_text(const entt::entity entity) {
    if (Logic_entt().all_of<Tag_UI_text_data_dirty, Tag_UI_text_renderable>(entity)) {
        draw_UI_text_impl(entity);
        logic_update_add_tag<Tag_UI_text>(entity);
        Logic_entt().remove<Tag_UI_text_data_dirty>(entity);
        return true;
    }
    return false;
}

/**
 * 任何时候 需要绘制的参数被修改之后会调用这个函数
 * 这个函数负责 触发 Tag_UI_text_data_dirty
 * Tag_UI_text_data_dirty 会调用上面的 draw_UI_text 函数
 * draw_UI_text 负责重新绘制，
 * @param entity
 * @return
 */
bool update_UI_text(const entt::entity entity) {
    if (Logic_entt().all_of<Tag_UI_text>(entity)) {
        Logic_entt().emplace_or_replace<Tag_UI_text_data_dirty>(entity);
        return true;
    }
    return false;
}
