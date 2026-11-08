//
// Created by 潘鑫 on 2026/4/9.
//
#include "UI/UI_text.h"


#include "utf8.h"
#include "json.hpp"  // 仅在需要处理具体业务的 .cpp 文件中引入完整实现
#include <fstream>

#include "UI/base_2d_render_object.h"
#include "bounding_box_2d.h"
#include "font_manager.h"
#include "render_component/Geometry_component.h"
#include "render_state.h"
#include "scene_component.h"
#include "UI/UI_button.h"

// void read_msdf_atlas(Msdf_text &msdf_text, std::string file_path) {
//     // 1. 打开文件流
//     std::ifstream file(file_path);
//
//     if (!file.is_open()) {
//         std::cerr << "无法打开文件！" << std::endl;
//         return;
//     }
//     try {
//         // 2. 直接从流解析
//         nlohmann::json data = nlohmann::json::parse(file);
//
//         if (data.contains("atlas")) {
//             auto atlas = data.at("atlas");
//             if (atlas.contains("type")) msdf_text.atlas.type = atlas.at("type");
//             if (atlas.contains("distanceRange")) msdf_text.atlas.distanceRange = atlas.at("distanceRange");
//             if (atlas.contains("distanceRangeMiddle"))
//                 msdf_text.atlas.distanceRangeMiddle = atlas.at("distanceRangeMiddle");
//             if (atlas.contains("size")) msdf_text.atlas.size = atlas.at("size");
//             if (atlas.contains("width")) msdf_text.atlas.width = atlas.at("width");
//             if (atlas.contains("height")) msdf_text.atlas.height = atlas.at("height");
//             if (atlas.contains("yOrigin")) msdf_text.atlas.yOrigin = atlas.at("yOrigin");
//         }
//         if (data.contains("metrics")) {
//             auto metrics = data.at("metrics");
//             if (metrics.contains("emSize")) msdf_text.metrics.emSize = metrics.at("emSize");
//             if (metrics.contains("lineHeight")) msdf_text.metrics.lineHeight = metrics.at("lineHeight");
//             if (metrics.contains("ascender")) msdf_text.metrics.ascender = metrics.at("ascender");
//             if (metrics.contains("descender")) msdf_text.metrics.descender = metrics.at("descender");
//             if (metrics.contains("underlineY")) msdf_text.metrics.underlineY = metrics.at("underlineY");
//             if (metrics.contains("underlineThickness"))
//                 msdf_text.metrics.underlineThickness = metrics.at("underlineThickness");
//         }
//
//         for (auto &glyph: data["glyphs"]) {
//             auto unicode = glyph.at("unicode");
//             struct Glyph tem;
//             tem.unicode = glyph.at("unicode");
//             tem.advance = glyph.at("advance");
//             if (glyph.contains("planeBounds")) {
//                 auto planeBounds       = glyph.at("planeBounds");
//                 tem.planeBounds.left   = planeBounds.at("left");
//                 tem.planeBounds.bottom = planeBounds.at("bottom");
//                 tem.planeBounds.right  = planeBounds.at("right");
//                 tem.planeBounds.top    = planeBounds.at("top");
//             }
//             if (glyph.contains("atlasBounds")) {
//                 auto atlasBounds       = glyph.at("atlasBounds");
//                 tem.atlasBounds.left   = atlasBounds.at("left");
//                 tem.atlasBounds.bottom = atlasBounds.at("bottom");
//                 tem.atlasBounds.right  = atlasBounds.at("right");
//                 tem.atlasBounds.top    = atlasBounds.at("top");
//             }
//             msdf_text.glyphs.insert({unicode, tem});
//         }
//     } catch (nlohmann::json::parse_error &e) {
//         std::cerr << "JSON 语法错误: " << e.what() << std::endl;
//     }
// }


void UI_Text_draw(const entt::entity entity) {
    auto &data = Logic_entt().get<UI_Text_data>(entity);

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


UI_Text::UI_Text(const std::string &debug_name) : object_2d(debug_name) {
    Logic_entt().emplace<UI_Text_data>(entity);
    // add_shader_path(VKR_shader_paths{
    //                     "2D/vulkan_MSDF_text",
    //                     "2D/vulkan_MSDF_text",
    //                     "", ""
    //                 });

    Logic_entt().emplace_or_replace<Tag_UI_text>(entity);
}

UI_Text &UI_Text::set_render_text(const std::string &utf8_text, const std::string &font_file_name, float char_size) {
    auto &data           = Logic_entt().get<UI_Text_data>(entity);
    data.render_text_    = utf8_text;
    data.font_file_name_ = font_file_name;
    data.char_size_      = char_size;
    Logic_entt().emplace_or_replace<Tag_UI_text_data_dirty>(entity);
    draw();
    return *this;
}


UI_Text &UI_Text::draw() {
    UI_Text_draw(entity);
    return *this;
}

void UI_Text::update() const {
    Logic_entt().emplace_or_replace<Tag_UI_text_data_dirty>(entity);
}

void UI_Text::set_renderable(bool renderable) const {
    if (renderable == true) {
        Logic_entt().emplace_or_replace<Tag_UI_text_renderable>(entity);
    } else {
        Logic_entt().remove<Tag_UI_text_renderable>(entity);
    }
}

void UI_Text::add_render_text(const std::string &text) {
    auto &data        = Logic_entt().get<UI_Text_data>(entity);
    data.render_text_ = data.render_text_ + text;
}

const std::string &UI_Text::get_render_text() const {
    auto &data = Logic_entt().get<UI_Text_data>(entity);
    return data.render_text_;
}
