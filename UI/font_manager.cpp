//
// Created by 潘鑫 on 2026/10/29.
//

#include "font_manager.h"


extern "C" {
#include <hb.h>
#include <hb-ft.h>
}

#include "bin_pack/MaxRectsBinPack.h"
#include "ext/import-font.h"
#include "LRU_cache.h"
#include "shader_common.h"
#include <msdfgen.h>
#include <msdfgen-ext.h> // 该头文件包含了加载字体所需的 FreetypeHandle

#include "Geometry_component.h"
#include "utf8/checked.h"


Font_Manager &Font_Manager::instance() {
    static Font_Manager *instance;

    static std::once_flag flag;
    std::call_once(flag, []() {
        instance = new Font_Manager();
        instance->create();
    });

    return *instance;
}

void Font_Manager::create() {
    atlas.type                = "msdf";
    atlas.distanceRange       = 4; // 距离过渡带的像素宽度
    atlas.distanceRangeMiddle = 0; // • 零交叉阈值（即字体的真实数学边界）。
    // 在采样距离场时，当解码出的数值恰好为 0，就代表这是字符的边缘。
    // 大于 0 的值代表在字形内部，小于 0 的值代表在字形外部。

    atlas.size    = 32;
    atlas.width   = 256;
    atlas.height  = 256;
    atlas.yOrigin = "bottom"; // 定义了纹理坐标的垂直方向

    // msdf_text.metrics.emSize             = 1;
    // msdf_text.metrics.lineHeight         = 1.3200000000000001;
    // msdf_text.metrics.ascender           = 1.02;
    // msdf_text.metrics.descender          = -0.29999999999999999;
    // msdf_text.metrics.underlineY         = -0.17999999999999999;
    // msdf_text.metrics.underlineThickness = 0.050000000000000003;

    image.init(static_cast<size_t>(atlas.width),
               static_cast<size_t>(atlas.height));

    // 有些字旋转了 90 度，有些字没有旋转，会导致复杂的 UV 坐标旋转矩阵传递 所以allowFlip 设置为 false
    bin_pack_manager.Init(static_cast<int>(atlas.width),
                          static_cast<int>(atlas.height), false);

    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
    }

    // 这里本质的调用是 FT_Init_FreeType ,  初始化一个 FT_Library library

    add_font("/Users/panxin/Library/Fonts/JetBrainsMonoNL-Regular.ttf");
}

void Font_Manager::destroy() {
    // 先清理掉这个
    if (nullptr != library)
        FT_Done_FreeType(library);
    library = nullptr;
}

void Font_Manager::check_need_update() {
    if (need_update_atlas == true) {
        // 需要更新图集 到 GPU , 在本帧开始绘制之前

    }
}

Texture_parameter Font_Manager::create_or_update_texture_from_image() {
    return create_texture_from_image(image.get_data(),
                                     image.get_width(),
                                     image.get_height(),
                                     4);
}

std::optional<msdfgen::Bitmap<float, 3> > generate_sdf_bitmap_and(Glyph &glyph, msdfgen::Shape &shape) {
    // 预处理：标准化轮廓方向
    shape.normalize();
    const auto bounds        = shape.getBounds();
    glyph.planeBounds.left   = static_cast<float>(bounds.l);
    glyph.planeBounds.right  = static_cast<float>(bounds.r);
    glyph.planeBounds.top    = static_cast<float>(bounds.t);
    glyph.planeBounds.bottom = static_cast<float>(bounds.b);
    edgeColoringByDistance(shape, 3.0);
    const float scale          = Font_Manager::instance().get_scale();         // 英文字母其实16就够了，行字需要 32;
    const double distanceRange = Font_Manager::instance().get_distanceRange(); // 让画布留白正好等于渐变宽度
    int width                  = static_cast<int>((bounds.r - bounds.l) * scale + 2 * distanceRange + 0.9999);
    int height                 = static_cast<int>((bounds.t - bounds.b) * scale + 2 * distanceRange + 0.9999);

    // 进位到偶数（部分图形 API 在渲染奇数宽度的纹理时性能较差）
    if (width % 2 != 0) width++;
    if (height % 2 != 0) height++;
    msdfgen::Bitmap<float, 3> msdf(width, height);
    const auto translate = msdfgen::Vector2(-bounds.l + distanceRange / scale,
                                            -bounds.b + distanceRange / scale);
    const msdfgen::SDFTransformation transform(msdfgen::Projection(scale, translate),
                                               msdfgen::Range(distanceRange / scale));

    msdfgen::MSDFGeneratorConfig config;
    config.overlapSupport                    = true; // 开启重叠支持
    config.errorCorrection.mode              = msdfgen::ErrorCorrectionConfig::EDGE_PRIORITY;
    config.errorCorrection.distanceCheckMode = msdfgen::ErrorCorrectionConfig::ALWAYS_CHECK_DISTANCE;

    generateMSDF(msdf, shape, transform, config);
    // Bitmap 中的数据生成好了
    return msdf;
}


template<typename T>
inline T clamp(T n) {
    return n >= T(0) && n <= T(1) ? n : T(n > T(0));
}

inline std::uint8_t pixelFloatToByte(float x) {
    return std::uint8_t(~int(255.5f - 255.f * clamp(x)));
}


void write_bitmap_to_bin_pack_image(const rbp::Rect &position, msdfgen::Bitmap<float, 3> &bit_map, Image &image) {
    for (auto j = 0; j < position.height; j++) {
        for (auto i = 0; i < position.width; i++) {
            const auto ptr = bit_map(i, position.height - 1 - j);
            image.write(position.x + i, position.y + j,
                        pixelFloatToByte(*(ptr + 0)),
                        pixelFloatToByte(*(ptr + 1)),
                        pixelFloatToByte(*(ptr + 2)));
        }
    }
}


bool Font_Manager::add_text_to_Atlas_and_bin_pack(const std::vector<uint32_t> &unicode_points,
                                                  const std::string &font_file_name) {
    // 这里有一个内存的泄漏
    // const auto font = msdfgen::loadFont(msdf_text_tem.freetype_handle, font_file_name.c_str());
    // if (font == nullptr) {
    //     // 字体加载失败
    // }
    // // 需要做的是添加一个缓冲，filename 先查找，如果有的话就添加在第一个，没有的话添加警告，选择其他的font
    // msdf_text_tem.fonts.push_back({font_file_name, font});
    auto &msdf_text_tem = get_font_data(font_file_name); // 也先空出来, 之后调用一个查找的函数就好
    auto font_handle    = msdfgen::adoptFreetypeFont(msdf_text_tem.font_handle);

    for (auto unicode_point: unicode_points)
        // 得到了每一个可能需要添加到 Atlas（图集） 中的文字
        if (msdf_text_tem.glyphs.find(unicode_point) == msdf_text_tem.glyphs.end()) {
            // 不存在,
            msdfgen::Shape shape;
            // 临时创建相应的 msdfgen::Shape 用于存储之后的 Contour（轮廓/闭合曲线）指的是构成一个 Glyph（字形）视觉形状的独立闭合线条回路
            if (loadGlyph(shape, font_handle, unicode_point, msdfgen::FONT_SCALING_EM_NORMALIZED)) {
                Glyph glyph;
                // 临时创建一个 Glyph 用于存储当前这个 字的
                glyph.unicode = unicode_point;
                auto bitmap   = generate_sdf_bitmap_and(glyph, shape);
                if (bitmap.has_value()) {
                    auto bit_map      = bitmap.value();
                    const auto width  = bitmap.value().width();
                    const auto height = bitmap.value().height();
                    // 下面一行是从图集中分配一个位置
                    auto position = bin_pack_manager.Insert(width, height,
                                                            rbp::MaxRectsBinPack::RectBestShortSideFit);
                    // 需要将 bitmap.value() 的内容写入图片 的 position
                    need_update_atlas = true;
                    //  这里的代码暂时不能分开, 如果直接申请固定大小的区域的话,是可以分开的
                    if (position.width == width) {
                        // 没有翻转
                        write_bitmap_to_bin_pack_image(position, bit_map, image);
                    } else {
                        // 翻转了长宽，顺时针旋转 90 度 // 顺逆都可以，确定同一个
                        assert(false);
                    }
                    // 也可能因为上下翻转需要处理一下下面的四个字
                    glyph.atlasBounds.left   = (static_cast<float>(position.x) + 0.5f);
                    glyph.atlasBounds.right  = (static_cast<float>(position.x + position.width) + 0.5f);
                    glyph.atlasBounds.top    = (static_cast<float>(position.y) + 0.5f);
                    glyph.atlasBounds.bottom = (static_cast<float>(position.y + position.height) + 0.5f);
                    msdf_text_tem.glyphs.insert({unicode_point, glyph});
                }
            }
        }
    msdfgen::destroyFont(font_handle);

    return true;
    // 还是先假设全部正常输出,
}

Font_Manager::Font_File_name_and_FontHandle &Font_Manager::get_font_data(const std::string &font_file_name) {
    for (auto &font: fonts) {
        if (font_file_name.compare(font_file_name) == 0) {
            return font;
        }
    }
    add_font(font_file_name);
    return fonts.back();
}


std::pair<std::shared_ptr<std::vector<Vertex_2D> >, std::shared_ptr<std::vector<unsigned short> > >
Font_Manager::create_text_geometry_data(const std::string &text,
                                        const std::string &font_file_name,
                                        const float start_position_x,
                                        const float start_position_y,
                                        const float max_position_x,
                                        const float max_position_y) {
    // 1. 创建缓冲区
    hb_buffer_t *buf = hb_buffer_create();

    // 2. 添加文本 (UTF-8 格式)
    hb_buffer_add_utf8(buf, text.c_str(), -1, 0, -1);
    // 3. 设置文本属性（必须指定，否则可能排版错误）

    hb_buffer_set_direction(buf, HB_DIRECTION_LTR);                 // 从左往右
    hb_buffer_set_script(buf, HB_SCRIPT_LATIN);                     // 拉丁脚本
    hb_buffer_set_language(buf, hb_language_from_string("en", -1)); // 英语

    // 使用 HarfBuzz 内置的 OpenType 支持加载字体
    // hb_blob_t *blob = hb_blob_create_from_file(font_file_name.c_str());
    // hb_face_t *face = hb_face_create(blob, 0);

    // 那么这里就只有一个问题,通过 font_file_name 获取 ftFace todo:
    FT_Face ftFace = get_font_data(font_file_name).font_handle;
    auto UPEM      = ftFace->units_per_EM;

    FT_Set_Pixel_Sizes(ftFace, 0, 16);
    // 还需要去设置字号的大小
    // 否则会缩成一团

    hb_font_t *font = hb_ft_font_create_referenced(ftFace);

    // 传入字体和缓冲区，执行塑造
    hb_shape(font, buf, NULL, 0);

    // 从缓冲区中提取字形索引和它们相对于基线的偏移量
    unsigned int glyph_count;
    hb_glyph_info_t *glyph_info    = hb_buffer_get_glyph_infos(buf, &glyph_count);
    hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);

    const auto vertices = std::make_shared<std::vector<Vertex_2D> >(); //  32  * 4 = 128
    const auto indices  = std::make_shared<std::vector<uint16_t> >();  //  2   * 6 = 12

    float char_size = 40;
    float current_x = start_position_x;
    float current_y = start_position_y + char_size;

    std::string utf8_text = text;
    std::vector<uint32_t> unicode_points;

    utf8::utf8to32(utf8_text.begin(), utf8_text.end(), std::back_inserter(unicode_points));

    for (unsigned int i = 0; i < unicode_points.size(); i++) {
        hb_codepoint_t glyph_id = unicode_points[i];
        // glyph_id 去map中查找索引
        // 得到的是一组相对于基线（Baseline）的数值
        const float x_advance = static_cast<float>(glyph_pos[i].x_advance) / static_cast<float>(UPEM); // 通常需要除以缩放系数
        const float y_advance = static_cast<float>(glyph_pos[i].y_advance) / static_cast<float>(UPEM);
        const float x_offset  = static_cast<float>(glyph_pos[i].x_offset) / static_cast<float>(UPEM);
        const float y_offset  = static_cast<float>(glyph_pos[i].x_offset) / static_cast<float>(UPEM);


        auto glyphs = get_font_data(font_file_name).glyphs;
        if (auto glyph = glyphs.find(glyph_id); glyph != glyphs.end()) {
            // 方向可能都稍微有点问题，但是结果是对的
            const auto char_uv = get_char_uv(glyph->second);
            append_text_box(vertices, indices, {
                                current_x + char_size * (x_offset + glyph->second.planeBounds.left),
                                current_y - char_size * (y_offset + glyph->second.planeBounds.bottom)
                            },
                            {
                                current_x + char_size * glyph->second.planeBounds.right,
                                current_y - char_size * glyph->second.planeBounds.top
                            },
                            char_uv);
            current_x += x_advance * char_size;
            current_y += y_advance * char_size;
        }


        // 使用这些数据配合渲染引擎（如 OpenGL/FreeType）绘制每一帧

        // 1. 步进值 (Advance) —— “笔尖移动了多少”
        // x_advance: 绘制完当前字形后，画笔（光标）应该在水平方向挪动多少距离。
        // y_advance: 垂直排版时画笔移动的距离。
        // 用途：它是决定下一个字画在哪里的核心依据。
        // 2. 偏移量 (Offset) —— “相对于基线的微调”
        // x_offset / y_offset: 有些字形（如阿拉伯语的变音符号）需要偏离标准位置。
        // 用途：在绘制当前字形时，给它一个临时的位移，但不影响下一个字的位置。

        // 用 HarfBuzz 的结果代替 JSON 的 advance 来更新光标位置
    }


    hb_buffer_destroy(buf);
    hb_font_destroy(font);

    return {vertices, indices};
}

Eigen::Vector4f Font_Manager::get_char_uv(const Glyph &glyph) const {
    return {
        (glyph.atlasBounds.left) / atlas.width,
        (glyph.atlasBounds.bottom) / atlas.height,
        (glyph.atlasBounds.right) / atlas.width,
        (glyph.atlasBounds.top) / atlas.height
    };
}
