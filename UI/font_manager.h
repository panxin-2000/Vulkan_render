//
// Created by 潘鑫 on 2026/10/29.
//

#ifndef HELLO_MAC_FONT_MANAGER_H
#define HELLO_MAC_FONT_MANAGER_H

#include <freetype/freetype.h>

#include "image.h"
#define  FT_LOAD_DEFAULT
#include <msdfgen.h>
#include <msdfgen-ext.h> // 该头文件包含了加载字体所需的 FreetypeHandle
#include <map>

#include "shader_common.h"
#include "vulkan_image.h"
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
// 字符级别的个体度量
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


class Font_Manager {
    using unicode_value  = uint32_t;
    using Font_File_name = std::string;

    struct Font_File_name_and_FontHandle {
        Font_File_name name;
        Metrics metrics; // 每一个字体的全局 Metrics 全都不一样
        FT_Face font_handle;
        std::map<unicode_value, Glyph> unicode_value_and_glyphs;

        // FT_Face 的对应封装句柄
        // 一个 FT_Face 代表了一个字体文件中的某一个“外观”
    };


    // FT_Face ft_face
    // 在 PC 中, 一般的做法是整个font 文件都读取放在内存中
    // 在嵌入式中, 一般的做法是 把 font 写如 SPI 存储芯片的固定位置,
    //           之后给出 font_name 与 具体地址的联系, 可以有多个 font_name
    //           然后就是怎么读取的问题了,
    //               外部内存直接映射（XIP 模式 —— 硬件级最强方案）
    //                  不消化 SRAM 内存的 , SPI控制器或 QSPI 控制器上本身 有 内存用来缓存
    //               FreeType 最高级的 FT_Open_Face 接口
    //                       通过纯软件的方式，为 FreeType 实现一个自定义的“流（Stream）”
    // 具体的哪里是最消耗执行时间的 代码, 并没有很去测试

    // freetype 是可以 自己生成 sdf 的 , 或者位图的
    // FreeType 是为了 PC 端设计的，内部有大量的 malloc 和 free ( 并不是很准确 )
    // 真正会产生大量临时分配的地方：字形缓存( Cache )
    //    如果你在渲染文本时发现内存分配频繁，通常不是 FreeType 核心解析器的锅，
    //    而是你开启了字形缓存（FT_Cache）或者高强度的矢量轮廓渲染（Outline Decomposition）
    //    FreeType 为了加速二次渲染，会把轮廓、位图缓存起来。如果不加限制，这个缓存池确实会频繁向你的内存管理器申请空间


public:
    // 理论上最外之需要调用两个函数
    //    create_text_geometry_data             在需要添加字体的时候调用
    //    create_or_update_texture_from_image   在逻辑线程完成向渲染线程同步前调用

    std::pair<std::shared_ptr<std::vector<Vertex_2D> >, std::shared_ptr<std::vector<unsigned short> > >
    create_text_geometry_data(const std::string &utf8_text,
                              const std::string &font_file,
                              float start_position_x,
                              float start_position_y, float max_position_x, float max_position_y);

    Texture_parameter create_or_update_texture_from_image();


    static Font_Manager &instance();

    void create();

    void destroy();

private:
    FT_Library library = nullptr;
    // FT_Library 的 对应封装句柄

    std::vector<Font_File_name_and_FontHandle> fonts;


    // LRU_cache<unicode_value, Glyph> unicode_cache;  // 想去添加这个 , 还需要把 字体 unicode_value 这两个作为索引去处理

    // 没有 LRU (Least Recently Used)可以考虑之后添加
    // 这三个是用来管理字体的存储在图像的哪里的 , 每次添加一个字都需要在渲染前进行更新
    Atlas atlas;                           // 纹理图集
    rbp::MaxRectsBinPack bin_pack_manager; //
    Image image;

    bool need_update_atlas = false;


    bool add_font(const Font_File_name &file_name);

    std::optional<msdfgen::Bitmap<float, 3> > generate_sdf_bitmap(Glyph &glyph, msdfgen::Shape &shape);

    bool add_text_to_Atlas_and_bin_pack(const std::vector<uint32_t> &unicode_points,
                                        const std::string &font_file_name);

    void clear_fonts();

    Font_File_name_and_FontHandle &get_font_data(const std::string &font_file_name);

    [[nodiscard]] float get_scale() const;

    [[nodiscard]] double get_distanceRange() const;


    /**
     * 检查是否有新的字被添加进行,需要更新图集
     */
    void check_need_update();

    Eigen::Vector4f get_char_uv(const Glyph &glyph) const;

private:
    static Font_Manager *instance_ptr;

    Font_Manager() = default;

    ~Font_Manager() = default;

    Font_Manager(const Font_Manager &) = delete;

    Font_Manager &operator=(const Font_Manager &) = delete;

    Font_Manager(Font_Manager &&) = delete;

    Font_Manager &operator=(Font_Manager &&) = delete;
};


#endif //HELLO_MAC_FONT_MANAGER_H
