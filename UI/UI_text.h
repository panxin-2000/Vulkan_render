//
// Created by 潘鑫 on 2026/4/8.
//

#ifndef HELLO_MAC_UI_TEXT_H
#define HELLO_MAC_UI_TEXT_H


#include "LRU_cache.h"
#include "MaxRectsBinPack.h"
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


class rect_Packing {
    // 需要装箱函数
    // 需要给出一个map,里面存储的是全部的空白的位置
    // 存储的是什么呢？ 一系列的空白box,
    struct uint_xy {
        uint x = 0;
        uint y = 0;

        bool operator<(const uint_xy &other) const {
            return std::tie(x, y) < std::tie(other.x, other.y);
        }
    };

    using start_point   = uint_xy;
    using size_of_block = uint_xy;

    std::map<size_of_block, start_point> boxs;


    start_point add_box(uint_xy box_size) {
        uint_xy start_position = {
            std::numeric_limits<uint>::max(),
            std::numeric_limits<uint>::max()
        };
        std::vector<std::pair<const uint_xy, uint_xy> *> keys;
        for (auto box: boxs) {
            if (box.first.x >= box_size.x && box.first.y >= box_size.y) {
                // 能够装箱
                keys.push_back(&box);
                // 直接更改值，
            }
        }
        return start_position;
    }
};


class Msdf_text {
public:
    Atlas atlas;
    Metrics metrics;
    using unicode_value = uint32_t;
    std::map<unicode_value, Glyph> glyphs;
    LRU_cache<unicode_value, Glyph> unicode_cache;

    // 没有 LRU (Least Recently Used)可以考虑之后添加
    rbp::MaxRectsBinPack texture_of_MSDF;

    Msdf_text(const size_t max_size) : unicode_cache(max_size) {
    }

    [[nodiscard]] float get_scale() const {
        return atlas.size;
    }

    [[nodiscard]] uint get_distanceRange() const {
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
