//
// Created by 潘鑫 on 2026/4/9.
//
#include "UI_text.h"

#include <hb.h>

#include "utf8.h"
#include "json.hpp"
#include <fstream>

void read_msdf_atlas(Msdf_text &msdf_text, std::string file_path) {
    // 1. 打开文件流
    std::ifstream file(file_path);

    if (!file.is_open()) {
        std::cerr << "无法打开文件！" << std::endl;
        return;
    }
    try {
        // 2. 直接从流解析
        nlohmann::json data = nlohmann::json::parse(file);

        if (data.contains("atlas")) {
            auto atlas = data.at("atlas");
            if (atlas.contains("type")) msdf_text.atlas.type = atlas.at("type");
            if (atlas.contains("distanceRange")) msdf_text.atlas.distanceRange = atlas.at("distanceRange");
            if (atlas.contains("distanceRangeMiddle"))
                msdf_text.atlas.distanceRangeMiddle = atlas.at("distanceRangeMiddle");
            if (atlas.contains("size")) msdf_text.atlas.size = atlas.at("size");
            if (atlas.contains("width")) msdf_text.atlas.width = atlas.at("width");
            if (atlas.contains("height")) msdf_text.atlas.height = atlas.at("height");
            if (atlas.contains("yOrigin")) msdf_text.atlas.yOrigin = atlas.at("yOrigin");
        }
        if (data.contains("metrics")) {
            auto metrics = data.at("metrics");
            if (metrics.contains("emSize")) msdf_text.metrics.emSize = metrics.at("emSize");
            if (metrics.contains("lineHeight")) msdf_text.metrics.lineHeight = metrics.at("lineHeight");
            if (metrics.contains("ascender")) msdf_text.metrics.ascender = metrics.at("ascender");
            if (metrics.contains("descender")) msdf_text.metrics.descender = metrics.at("descender");
            if (metrics.contains("underlineY")) msdf_text.metrics.underlineY = metrics.at("underlineY");
            if (metrics.contains("underlineThickness"))
                msdf_text.metrics.underlineThickness = metrics.at("underlineThickness");
        }

        for (auto &glyph: data["glyphs"]) {
            auto unicode = glyph.at("unicode");
            struct Glyph tem;
            tem.unicode = glyph.at("unicode");
            tem.advance = glyph.at("advance");
            if (glyph.contains("planeBounds")) {
                auto planeBounds       = glyph.at("planeBounds");
                tem.planeBounds.left   = planeBounds.at("left");
                tem.planeBounds.bottom = planeBounds.at("bottom");
                tem.planeBounds.right  = planeBounds.at("right");
                tem.planeBounds.top    = planeBounds.at("top");
            }
            if (glyph.contains("atlasBounds")) {
                auto atlasBounds       = glyph.at("atlasBounds");
                tem.atlasBounds.left   = atlasBounds.at("left");
                tem.atlasBounds.bottom = atlasBounds.at("bottom");
                tem.atlasBounds.right  = atlasBounds.at("right");
                tem.atlasBounds.top    = atlasBounds.at("top");
            }
            msdf_text.glyphs.insert({unicode, tem});
        }
    } catch (nlohmann::json::parse_error &e) {
        std::cerr << "JSON 语法错误: " << e.what() << std::endl;
    }
}


void create_text_render(const entt::entity entity, const std::string &name, Msdf_text &msdf_text,
                        float min_x,
                        float min_y) {
    // 1. 创建缓冲区
    hb_buffer_t *buf = hb_buffer_create();

    // 2. 添加文本 (UTF-8 格式)
    hb_buffer_add_utf8(buf, name.c_str(), -1, 0, -1);
    // 3. 设置文本属性（必须指定，否则可能排版错误）

    hb_buffer_set_direction(buf, HB_DIRECTION_LTR);                 // 从左往右
    hb_buffer_set_script(buf, HB_SCRIPT_LATIN);                     // 拉丁脚本
    hb_buffer_set_language(buf, hb_language_from_string("en", -1)); // 英语

    // 使用 HarfBuzz 内置的 OpenType 支持加载字体
    hb_blob_t *blob = hb_blob_create_from_file("/Users/panxin/Library/Fonts/JetBrainsMonoNL-Regular.ttf");
    hb_face_t *face = hb_face_create(blob, 0);
    hb_font_t *font = hb_font_create(face);
    auto UPEM       = hb_face_get_upem(face);

    // 传入字体和缓冲区，执行塑造
    hb_shape(font, buf, NULL, 0);

    // 从缓冲区中提取字形索引和它们相对于基线的偏移量
    unsigned int glyph_count;
    hb_glyph_info_t *glyph_info    = hb_buffer_get_glyph_infos(buf, &glyph_count);
    hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);

    const auto vertices = std::make_shared<std::vector<Vertex> >();   //  32  * 4 = 128
    const auto indices  = std::make_shared<std::vector<uint16_t> >(); //  2   * 6 = 12

    float char_size = 40;
    float current_x = min_x;
    float current_y = min_y + char_size;

    std::string utf8_text = name;
    std::vector<uint32_t> unicode_points;

    utf8::utf8to32(utf8_text.begin(), utf8_text.end(), std::back_inserter(unicode_points));

    for (unsigned int i = 0; i < unicode_points.size(); i++) {
        hb_codepoint_t glyph_id = unicode_points[i];
        // glyph_id 去map中查找索引
        // 得到的是一组相对于基线（Baseline）的数值
        float x_advance = (float) glyph_pos[i].x_advance / (float) UPEM; // 通常需要除以缩放系数
        float y_advance = (float) glyph_pos[i].y_advance / (float) UPEM;
        float x_offset  = (float) glyph_pos[i].x_offset / (float) UPEM;
        float y_offset  = (float) glyph_pos[i].x_offset / (float) UPEM;


        auto glyph = msdf_text.glyphs.find(glyph_id);
        if (glyph != msdf_text.glyphs.end()) {
            // 方向可能都稍微有点问题，但是结果是对的
            add_text_box(vertices, indices, {
                             current_x + char_size * (x_offset + glyph->second.planeBounds.left),
                             current_y - char_size * (y_offset + glyph->second.planeBounds.bottom),
                             0
                         },
                         {
                             current_x + char_size * glyph->second.planeBounds.right,
                             current_y - char_size * glyph->second.planeBounds.top,
                             0
                         },
                         (glyph->second.atlasBounds.left) / msdf_text.atlas.width,
                         (msdf_text.atlas.height - glyph->second.atlasBounds.bottom) / msdf_text.atlas.height,
                         (glyph->second.atlasBounds.right) / msdf_text.atlas.width,
                         (msdf_text.atlas.height - glyph->second.atlasBounds.top) / msdf_text.atlas.height);
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
    add_geometry_data(entity, vertices, indices);


    hb_buffer_destroy(buf);
    hb_font_destroy(font);
    hb_face_destroy(face);
    hb_blob_destroy(blob);
}


Msdf_text *msdf_text = nullptr;

Msdf_text &get_msdf_text() {
    if (msdf_text == nullptr) {
        msdf_text                            = new Msdf_text();
        msdf_text->atlas.type                = "msdf";
        msdf_text->atlas.distanceRange       = 4;
        msdf_text->atlas.distanceRangeMiddle = 0;
        msdf_text->atlas.size                = 32;
        msdf_text->atlas.width               = 2048;
        msdf_text->atlas.height              = 2048;
        msdf_text->atlas.yOrigin             = "bottom";

        msdf_text->metrics.emSize             = 1;
        msdf_text->metrics.lineHeight         = 1.3200000000000001;
        msdf_text->metrics.ascender           = 1.02;
        msdf_text->metrics.descender          = -0.29999999999999999;
        msdf_text->metrics.underlineY         = -0.17999999999999999;
        msdf_text->metrics.underlineThickness = 0.050000000000000003;

        return *msdf_text;
    } else {
        return *msdf_text;
    }
}

#include <msdfgen.h>
#include <msdfgen-ext.h> // 该头文件包含了加载字体所需的 FreetypeHandle

Msdf_text &get_msdf_text_add_string(const std::string &name) {
    std::string utf8_text = name;
    std::vector<uint32_t> unicode_points;
    utf8::utf8to32(utf8_text.begin(), utf8_text.end(), std::back_inserter(unicode_points));
    auto msdf_text = get_msdf_text();

    msdfgen::FreetypeHandle *ft = msdfgen::initializeFreetype();

    msdfgen::FontHandle *font = loadFont(ft, "/Users/panxin/Library/Fonts/JetBrainsMonoNL-Regular.ttf");
    if (!font) {
        deinitializeFreetype(ft);
        return msdf_text;
    }

    for (auto unicode_point: unicode_points) {
        // 需要
        if (msdf_text.glyphs.find(unicode_point) != msdf_text.glyphs.end()) {
        } else {
            Glyph glyph;
            glyph.unicode = unicode_point;
            msdfgen::Shape shape;
            if (loadGlyph(shape, font, unicode_point, msdfgen::FONT_SCALING_EM_NORMALIZED)) {
                // 预处理：标准化轮廓方向
                shape.normalize();
                auto bounds              = shape.getBounds();
                glyph.planeBounds.left   = static_cast<float>(bounds.l);
                glyph.planeBounds.right  = static_cast<float>(bounds.r);
                glyph.planeBounds.top    = static_cast<float>(bounds.t);
                glyph.planeBounds.bottom = static_cast<float>(bounds.b);

                // 为边分配颜色（MSDF 的核心步骤，确保角点锐利）
                edgeColoringByDistance(shape, 3.0);

                float size_of_msdf = 32;
                float scale        = 32;
                double padding     = 2.0;

                // 4. 配置输出位图 (32x32 像素)
                msdfgen::Bitmap<float, 3> msdf(size_of_msdf, size_of_msdf);
                int width  = static_cast<int>((bounds.r - bounds.l) * scale + 2 * padding + 0.9999);
                int height = static_cast<int>((bounds.t - bounds.b) * scale + 2 * padding + 0.9999);

                // 2. 进位到偶数（部分图形 API 在渲染奇数宽度的纹理时性能较差）
                if (width % 2 != 0) width++;
                if (height % 2 != 0) height++;
                // width 和 height 就是需要排列的盒子，装箱算法中需要放置的箱子
                // 只剩下装箱需要去管理了

                // 5. 设置投影变换 (缩放和位移)
                // 参数：Projection(scale, translation), range (边缘影响范围)
                msdfgen::SDFTransformation transform(
                                                     msdfgen::Projection(size_of_msdf,
                                                                         msdfgen::Vector2(7.0 / size_of_msdf,
                                                                                  4.0 / size_of_msdf + padding /
                                                                                  size_of_msdf)),
                                                     msdfgen::Range(4.0 / size_of_msdf));

                // 推荐设置：range = 2.0
                // 如果要加外发光/描边：可以设为 4.0 或更高，因为你需要额外的空间来存储边缘之外的距离信息。
                // 6. 执行 MSDF 生成核心算法

                msdfgen::MSDFGeneratorConfig config;
                config.overlapSupport                    = true; // 开启重叠支持
                config.errorCorrection.mode              = msdfgen::ErrorCorrectionConfig::EDGE_PRIORITY;
                config.errorCorrection.distanceCheckMode = msdfgen::ErrorCorrectionConfig::ALWAYS_CHECK_DISTANCE;

                generateMSDF(msdf, shape, transform, config);

                // overlapSupport (bool)：
                // 描述：是否开启重叠支持（默认为 true）。
                // 作用：如果矢量路径中存在重叠的轮廓（Contours），该参数可以确保距离场计算的正确性。
                // A 的 上面 确实是重叠的路径 ，主要还是指向了这个


                // 将 msdf 转换为 0-1，然后再上传到 GPU  也可以直接上传，之后再到 GPU 中 调用计算着色器做转移
                // float range = 2.0f; // 必须与生成时设置的 range 一致
                // float dist = pixelValue; // 来自 Bitmap<float, 3> 的值
                // // 1. 归一化到 [0, 1]
                // float normalized = dist / range + 0.5f;
                // // 2. 截断并映射到 [0, 255]
                // unsigned char out = (unsigned char)std::max(0.0f, std::min(255.0f, normalized * 255.0f + 0.5f));

                // 想要实现单个字体的替换更新
                // 字符排版管理器 (Packer)
                // 动态 LRU 缓存系统
                // GPU 纹理更新 (Incremental Updates)


                // 7. 保存为 PNG 文件 (需要链接 msdfgen-ext)
                savePng(msdf, "output_A_msdf.png");
                std::cout << "MSDF image generated successfully!" << std::endl;
                // 因为是需要输出，所以不能save Png,需要写入到其他位置
            }

            msdf_text.glyphs.insert({unicode_point, glyph});
        }
    }
}


entt::entity UI_text(const std::string &name,
                     float min_x,
                     float min_y,
                     float max_x,
                     float max_y) {
    std::string_view df = "";

    LOG_INFO(g_log(), "UI create  {} {} {} {} {} ", name, min_x, min_y, max_x, max_y);

    const entt::entity entity = Logic_entt().create();
    Logic_entt().emplace<Proxy_entity>(entity, Render_entt().create());


    Logic_entt().emplace<Rect_2D_transform>(entity);
    Logic_entt().emplace<VKR_shader_paths>(entity,
                                           "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_MSDF_text.vert.spv",
                                           "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_MSDF_text.frag.spv",
                                           "", "");

    if (auto *scene_node = Logic_entt().try_get<Rect_2D_transform>(entity)) {
        scene_node->set_bounding_box({min_x, min_y}, {max_x, max_y});
    }
    Logic_entt().emplace<Drag_event>(entity);
    Logic_entt().emplace<Name_component>(entity, name);

    Msdf_text msdf_text;
    read_msdf_atlas(msdf_text, "atlas.json");
    create_text_render(entity, name, msdf_text, min_x, min_y); // 如果可以，尽量考虑圆角部分的内容
    // 不同的材质？ 不同的着色器

    matrix_4x4 model;
    UI_matrix_4x4(&model, {1, 1}, {0, 0});
    set_render_parameter(entity, "model_4x4", model);

    Logic_entt().emplace_or_replace<add_to_render_tag>(entity);
    Logic_entt().emplace_or_replace<UI_2D_tag>(entity);

    scene_root_add_child(entity);
    return entity;

    // 还想需要添加位置的，以及缩放。缩放暂时不需要，需要添加层。
    /***************添加到渲染管理器**********************/
}
