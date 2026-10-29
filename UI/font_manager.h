//
// Created by 潘鑫 on 2026/10/29.
//

#ifndef HELLO_MAC_FONT_MANAGER_H
#define HELLO_MAC_FONT_MANAGER_H

#include "image.h"
#include <msdfgen.h>
#include <msdfgen-ext.h> // 该头文件包含了加载字体所需的 FreetypeHandle
#include <map>

#include "shader_common.h"
#include "bin_pack/MaxRectsBinPack.h"

// 纹理图集 Texture Atlas
struct Atlas {
    std::string type;
    float distanceRange       = 0.0f;
    float distanceRangeMiddle = 0.0f;
    float size                = 0.0f;
    float width               = 0.0f;
    float height              = 0.0f;
    std::string yOrigin;
};

// 字形/图形度量信息
struct Metrics {
    float emSize             = 0.0f; // 基准单位
    float lineHeight         = 0.0f; // 下一行的起始位置 字体大小 * lineHeight
    float ascender           = 0.0f; // 字符（如 'h' 或 'A'）从基线向上延伸的最大距离
    float descender          = 0.0f; // 字符（如 'g' 或 'p'）掉到基线以下的最大深度
    float underlineY         = 0.0f; // 下划线的垂直位置， 下划线位于基线 下方 underlineY 个单位处
    float underlineThickness = 0.0f; // 下划线的粗细
};

// 字母 “A” 的线条、轮廓和像素图像
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


class Msdf_text {
public:
    Atlas atlas;
    Metrics metrics;
    using unicode_value = uint32_t;
    std::map<unicode_value, Glyph> glyphs;
    // LRU_cache<unicode_value, Glyph> unicode_cache;

    // 没有 LRU (Least Recently Used)可以考虑之后添加
    rbp::MaxRectsBinPack bin_pack_manager;
    Image image;

    msdfgen::FreetypeHandle *ft = nullptr;
    using Font_File_name        = std::string;

    struct Font_File_name_and_FontHandle {
        Font_File_name name;
        msdfgen::FontHandle *font_handle;
    };

    std::vector<Font_File_name_and_FontHandle> fonts;

    Msdf_text() = default;

    ~Msdf_text() {
        for (const auto &font: fonts) {
            free(font.font_handle);
        }
    }

    bool add_font(const Font_File_name &file_name) {
        if (ft != nullptr) {
            msdfgen::FontHandle *font = msdfgen::loadFont(ft, file_name.c_str());
            if (font != nullptr) {
                fonts.push_back({file_name.c_str(), font});
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] float get_scale() const {
        return atlas.size;
    }

    [[nodiscard]] uint get_distanceRange() const {
        return static_cast<int>(atlas.distanceRange);
    }
};


class Font_Manager {
public:
    Msdf_text msdf_text;

public:
    Font_Manager(const Font_Manager &) = delete;

    Font_Manager &operator=(const Font_Manager &) = delete;

    Font_Manager(Font_Manager &&) = delete;

    Font_Manager &operator=(Font_Manager &&) = delete;

    static Font_Manager &instance();

    void create();

private:
    Font_Manager() = default;

    ~Font_Manager() = default;
};

bool add_text_to_Atlas_and_bin_pack(Msdf_text &msdf_text_tem,
                                    const std::vector<uint32_t> &unicode_points,
                                    const std::string &font_file_name);


std::pair<std::shared_ptr<std::vector<Vertex_2D> >, std::shared_ptr<std::vector<unsigned short> > >
create_text_geometry_data(
    const std::string &text,
    std::map<uint32_t, Glyph> &glyphs,
    float min_x,
    float min_y,
    float atlas_width,
    float atlas_height);


#endif //HELLO_MAC_FONT_MANAGER_H
