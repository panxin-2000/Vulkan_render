//
// Created by 潘鑫 on 2026/4/8.
//

#ifndef HELLO_MAC_UI_TEXT_H
#define HELLO_MAC_UI_TEXT_H


#include "name_component.h"
#include "Rect_2D_component.h"


struct Atlas {
    std::string type;
    float distanceRange       = 0.0f;
    float distanceRangeMiddle = 0.0f;
    float size                = 0.0f;
    float width               = 0.0f;
    float height              = 0.0f;
    std::string yOrigin;
};

struct Metrics {
    float emSize             = 0.0f; // 基准单位
    float lineHeight         = 0.0f; // 下一行的起始位置 字体大小 * lineHeight
    float ascender           = 0.0f; // 字符（如 'h' 或 'A'）从基线向上延伸的最大距离
    float descender          = 0.0f; // 字符（如 'g' 或 'p'）掉到基线以下的最大深度
    float underlineY         = 0.0f; // 下划线的垂直位置， 下划线位于基线 下方 underlineY 个单位处
    float underlineThickness = 0.0f; // 下划线的粗细
};


struct Glyph {
    uint32_t unicode = 0;
    float advance    = 0.6f; // 表示渲染完这个字符后，光标应该向右移动多远来放置下一个字符

    struct direction {
        float left   = 0.0f;
        float bottom = 0.0f;
        float right  = 0.0f;
        float top    = 0.0f;
    };

    direction planeBounds; // 描述该字符在逻辑空间（渲染画布）中的形状范围
    direction atlasBounds; // 描述该字符在实际图片文件（纹理贴图）中的像素坐标
};

struct Msdf_text {
    Atlas atlas;
    Metrics metrics;
    using unicode_value = uint32_t;
    std::map<unicode_value, Glyph> glyphs;

public:
    const float get_scale() const {
        return atlas.size;
    }

    const uint get_distanceRange() const {
        return static_cast<int>(atlas.distanceRange);
    }
};


void create_text_render(const entt::entity entity, const std::string &name, Msdf_text &msdf_text,
                        float min_x,
                        float min_y);

void read_msdf_atlas(Msdf_text &msdf_text, std::string file_path);

entt::entity UI_text(const std::string &name,
                     float min_x,
                     float min_y,
                     float max_x,
                     float max_y);


#endif //HELLO_MAC_UI_TEXT_H
