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

Font_Manager *Font_Manager::instance_ptr = nullptr;


Font_Manager &Font_Manager::instance() {
    static std::once_flag flag;
    std::call_once(flag, []() {
        instance_ptr = new Font_Manager();
        instance_ptr->create();
    });

    return *instance_ptr;
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

    delete instance_ptr;
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

std::optional<msdfgen::Bitmap<float, 3> > Font_Manager::generate_sdf_bitmap(Glyph &glyph, msdfgen::Shape &shape) {
    // 预处理：标准化轮廓方向
    if (shape.contours.empty()) {
        return {};
    }
    shape.normalize();
    const auto bounds        = shape.getBounds();
    glyph.planeBounds.left   = static_cast<float>(bounds.l);
    glyph.planeBounds.right  = static_cast<float>(bounds.r);
    glyph.planeBounds.top    = static_cast<float>(bounds.t);
    glyph.planeBounds.bottom = static_cast<float>(bounds.b);
    edgeColoringByDistance(shape, 3.0);
    const float scale          = get_scale();         // 英文字母其实16就够了，行字需要 32;
    const double distanceRange = get_distanceRange(); // 让画布留白正好等于渐变宽度
    // 1. 先计算完美的、未取整的浮点数尺寸
    double exactWidth  = (bounds.r - bounds.l) * scale + 2 * distanceRange + 0.999f;
    double exactHeight = (bounds.t - bounds.b) * scale + 2 * distanceRange + 0.999f;

    // 2. 转换为 int 并进位到偶数
    int width  = static_cast<int>(exactWidth);
    int height = static_cast<int>(exactHeight);

    // 进位到偶数（部分图形 API 在渲染奇数宽度的纹理时性能较差）
    if (width % 2 != 0) width++;
    if (height % 2 != 0) height++;

    // 3. 计算由于进位（以及 static_cast 截断）导致纹理多出来的物理像素
    double extraPixelX = static_cast<double>(width) - exactWidth;
    double extraPixelY = static_cast<double>(height) - exactHeight;

    // 4. 将多出来的像素平均分配到两边（除以 2），并转换回字形设计单位（除以 scale）
    double paddingOffsetX = (extraPixelX / 2.0) / scale;
    double paddingOffsetY = (extraPixelY / 2.0) / scale;

    // 5. 在原本的 translate 基础上，加上这个居中补偿量
    const auto translate = msdfgen::Vector2(-bounds.l + distanceRange / scale + paddingOffsetX,
                                            -bounds.b + distanceRange / scale + paddingOffsetY);

    msdfgen::Bitmap<float, 3> msdf(width, height);
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

bool Font_Manager::add_font(const Font_File_name &file_name) { {
        FT_Face face;
        FT_Error error = FT_New_Face(library, file_name.c_str(), 0, &face);
        if (error) {
            return false;
        }
        msdfgen::FontHandle *font = msdfgen::adoptFreetypeFont(face);
        // 上一行的本质调用是 FT_New_Face  , 目的是为了 初始化 FT_Face face
        // msdfgen::FontHandle* font = msdfgen::adoptFreetypeFont(ftFace); // 另一个做法,也是可以使用的
        // 那么正确的建议就是是 使用 ftFace , 以它作为基础来管理各个字体
        // FT_New_Face , 调用这个函数之后再调用 msdfgen::adoptFreetypeFont  就可以了

        if (font != nullptr) {
            msdfgen::FontMetrics metrics;
            //  调用 getFontMetrics 读取度量数据
            // 第三个参数指定缩放类型，建议使用 FONT_SCALING_EM_NORMALIZED
            // 这样获取到的就是您前面看到的那种以 EM = 1 为基准的归一化浮点数
            bool success = msdfgen::getFontMetrics(metrics, font, msdfgen::FONT_SCALING_EM_NORMALIZED);
            if (success) {
                // 5. 成功读取后，直接从结构体中提取您需要的固定值
                // std::cout << "Line Height (推荐行高): " << metrics.lineHeight << std::endl;
                // std::cout << "Ascender (上伸高度): " << metrics.ascenderY << std::endl;
                // std::cout << "Descender (下伸高度): " << metrics.descenderY << std::endl;
                // std::cout << "Underline Y (下划线Y): " << metrics.underlineY << std::endl;
                // std::cout << "Underline Thickness (下划线厚度): " << metrics.underlineThickness << std::endl;

                Metrics metrics_temp;
                metrics_temp.emSize             = metrics.emSize;
                metrics_temp.lineHeight         = metrics.lineHeight;
                metrics_temp.ascender           = metrics.ascenderY;
                metrics_temp.descender          = metrics.descenderY;
                metrics_temp.underlineY         = metrics.underlineY;
                metrics_temp.underlineThickness = metrics.underlineThickness;
                fonts.push_back({file_name.c_str(), metrics_temp, face});
            } else {
                std::cerr << "获取 Metrics 失败！" << std::endl;
            }
            msdfgen::destroyFont(font);
            return true;
        }
    }
    return false;
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
        if (msdf_text_tem.unicode_value_and_glyphs.find(unicode_point) == msdf_text_tem.unicode_value_and_glyphs.
            end()) {
            // 不存在,
            msdfgen::Shape shape;
            // 临时创建相应的 msdfgen::Shape 用于存储之后的 Contour（轮廓/闭合曲线）指的是构成一个 Glyph（字形）视觉形状的独立闭合线条回路
            if (loadGlyph(shape, font_handle, unicode_point, msdfgen::FONT_SCALING_EM_NORMALIZED)) {
                // FT_Load_Glyph  这里是 freetype 中调用的,  loadGlyph 封装了这个逻辑
                shape.orientContours();
                Glyph glyph;
                // 临时创建一个 Glyph 用于存储当前这个 字的
                glyph.unicode = unicode_point;
                auto bitmap   = generate_sdf_bitmap(glyph, shape);
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
                    msdf_text_tem.unicode_value_and_glyphs.insert({unicode_point, glyph});
                } else {
                    msdf_text_tem.unicode_value_and_glyphs.insert({unicode_point, glyph});
                }
            }
        }
    msdfgen::destroyFont(font_handle);

    return true;
    // 还是先假设全部正常输出,
}

void Font_Manager::clear_fonts() {
    for (const auto &font: fonts) {
        FT_Done_Face(font.font_handle);
    }
    fonts.clear();
}

Font_Manager::Font_File_name_and_FontHandle &Font_Manager::get_font_data(const std::string &font_file_name) {
    for (auto &font: fonts) {
        if (font.name.compare(font_file_name) == 0) {
            return font;
        }
    }
    add_font(font_file_name);
    return fonts.back();
}

float Font_Manager::get_scale() const {
    return atlas.size;
}

double Font_Manager::get_distanceRange() const {
    return static_cast<double>(atlas.distanceRange);
}


std::pair<std::shared_ptr<std::vector<Vertex_2D> >, std::shared_ptr<std::vector<unsigned short> > >
Font_Manager::create_text_geometry_data(const std::string &utf8_text,
                                        const std::string &font_file,
                                        const float start_position_x,
                                        const float start_position_y,
                                        const float max_position_x,
                                        const float max_position_y,
                                        const float char_size) {
    std::string font_file_name;
    if (font_file.empty()) {
        font_file_name = "default_font";
    } else {
        font_file_name = font_file;
    }


    std::vector<uint32_t> unicode_vector;
    utf8::utf8to32(utf8_text.begin(), utf8_text.end(), std::back_inserter(unicode_vector));
    add_text_to_Atlas_and_bin_pack(unicode_vector, font_file_name);


    // 1. 创建缓冲区
    hb_buffer_t *buf = hb_buffer_create();

    // 2. 添加文本 (UTF-8 格式)
    hb_buffer_add_utf8(buf, utf8_text.c_str(), -1, 0, -1);
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

    // 还需要去设置字号的大小
    // 否则会缩成一团

    hb_font_t *font = hb_ft_font_create_referenced(ftFace);

    hb_font_set_scale(font, char_size * 64, char_size * 64);
    // 传入字体和缓冲区，执行塑造
    hb_shape(font, buf, NULL, 0);

    // 从缓冲区中提取字形索引和它们相对于基线的偏移量
    unsigned int glyph_count;
    hb_glyph_info_t *glyph_info    = hb_buffer_get_glyph_infos(buf, &glyph_count);
    hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);

    const auto vertices = std::make_shared<std::vector<Vertex_2D> >(); //  32  * 4 = 128
    const auto indices  = std::make_shared<std::vector<uint16_t> >();  //  2   * 6 = 12

    float current_x = start_position_x;
    float current_y = start_position_y + char_size;
    unsigned int x_ppem;
    unsigned int y_ppem;
    hb_font_get_ppem(font, &x_ppem, &y_ppem);


    for (unsigned int i = 0; i < unicode_vector.size(); i++) {
        hb_codepoint_t glyph_id = unicode_vector[i];
        // glyph_id 去map中查找索引
        // 得到的是一组相对于基线（Baseline）的数值
        const float x_advance = static_cast<float>(glyph_pos[i].x_advance) / static_cast<float>(64); // 通常需要除以缩放系数
        const float y_advance = static_cast<float>(glyph_pos[i].y_advance) / static_cast<float>(64);
        const float x_offset  = static_cast<float>(glyph_pos[i].x_offset) / static_cast<float>(64);
        const float y_offset  = static_cast<float>(glyph_pos[i].x_offset) / static_cast<float>(64);


        auto &temp_map = get_font_data(font_file_name).unicode_value_and_glyphs;
        if (auto value = temp_map.find(glyph_id); value != temp_map.end()) {
            // 方向可能都稍微有点问题，但是结果是对的
            const auto char_uv = get_char_uv(value->second);
            append_text_box(vertices, indices, {
                                current_x + char_size * (value->second.planeBounds.left),
                                current_y - char_size * (value->second.planeBounds.bottom)
                            },
                            {
                                current_x + char_size * value->second.planeBounds.right,
                                current_y - char_size * value->second.planeBounds.top
                            },
                            char_uv);
            current_x += x_advance;
            current_y += y_advance;
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
        (glyph.atlasBounds.left + 2) / atlas.width,
        (glyph.atlasBounds.bottom - 2) / atlas.height,
        (glyph.atlasBounds.right - 2) / atlas.width,
        (glyph.atlasBounds.top + 2) / atlas.height
    };
}
