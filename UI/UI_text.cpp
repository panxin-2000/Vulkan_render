//
// Created by 潘鑫 on 2026/4/9.
//
#include "UI_text.h"


#include "utf8.h"
#include "json.hpp"  // 仅在需要处理具体业务的 .cpp 文件中引入完整实现
#include <fstream>

#include "base_2d_render_object.h"
#include "bounding_box_2d.h"
#include "render_component/Geometry_component.h"
#include "model_matrix.h"

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


UI_Text::UI_Text(const std::string &name) : object_2d(name) {
    Logic_entt().emplace<UI_Text_data>(entity);
    add_shader_path(VKR_shader_paths{
                        "2D/vulkan_MSDF_text",
                        "2D/vulkan_MSDF_text",
                        "", ""
                    });
    int logical_w, logical_h;
    const auto &backend = VK_backend::instance();

    logic_update_add_tag<UI_render_text>(entity);

    SDL_GetWindowSize(backend.get_window(), &logical_w, &logical_h);

    Eigen::Vector2f scale{2.0f / logical_w, 2.0f / logical_h};
    Eigen::Vector2f translate{-1.0f - 0 * scale[0], -1.0f - 0 * scale[1]};

    add_push_constant_parameter("uScale", scale);
    add_push_constant_parameter("uTranslate", translate);
}

void UI_Text::set_render_text(const std::string &utf8_text, const std::string &font_file_name, float char_size) {
    auto &data           = Logic_entt().get<UI_Text_data>(entity);
    data.render_text_    = utf8_text;
    data.font_file_name_ = font_file_name;
    data.char_size_      = char_size;
    update();
}


void update_UI_Text(const entt::entity entity) {
    auto &data = Logic_entt().get<UI_Text_data>(entity);

    float min_x = 0;
    float min_y = 0;
    float max_x = 0;
    float max_y = 0;
    if (auto parent = get_parent(entity); parent != entt::null) {
        if (Logic_entt().all_of<Bounding_Box_2D>(parent)) {
            const auto &parent_bounding_box = Logic_entt().get<Bounding_Box_2D>(parent);
            min_x                           = parent_bounding_box.box[0];
            min_y                           = parent_bounding_box.box[1];
            max_x                           = parent_bounding_box.box[2];
            max_y                           = parent_bounding_box.box[3];
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


    set_geometry_data(entity, result.first, result.second);
    logic_update_proxy(entity, get_VKR_mesh(entity));
    logic_update_proxy(entity, create_primitives(entity));
    logic_update_add_tag<UI_render_text>(entity);
}

void UI_Text::update() {
    update_UI_Text(entity);
}
