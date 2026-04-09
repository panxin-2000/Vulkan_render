/* Copyright (c) 2025-2026, Sascha Willems
 * SPDX-License-Identifier: MIT
 */

#include <GLFW/glfw3.h>

#include <thread>
#include "render_thread/backend.h"
#include "vulkan_backend.h"
#include "event/base_event.h"
#include "labyrinth.h"
#include "UI/UI_block.h"
#include "UI/UI_button.h"

#include "global_singleton.h"
#include "descriptor_pool.h"
#include "earcut.h"
#include "sync_proxy_to_render_thread.h"
#include "update_push_constants_data.h"
#include "vk_render_to_image.h"
#include "vulkan_sample.h"
#include "ccd/ccd.h"
#include "manifold/cross_section.h"
#include "manifold/manifold.h"
#include "UI/3d_model_display.h"
#include "UI/UI_text.h"

void register_glfw(GLFWwindow *window);

void deal_glfw_event();


inline entt::entity add_render_pass(const std::string &name) {
    entt::entity entity = Logic_entt().create();
    Logic_entt().emplace<Proxy_entity>(entity, Render_entt().create());

    Logic_entt().emplace<Name_component>(entity, name + "deferred_pass");

    Logic_entt().emplace<VKR_shader_paths>(entity,
                                           "/Users/panxin/CLionProjects/hello_mac/render/shader/deferred.vert.spv",
                                           "/Users/panxin/CLionProjects/hello_mac/render/shader/deferred.frag.spv",
                                           "", "");

    // 更新物体的模型矩阵

    world_root_add_child(entity);

    Logic_entt().emplace_or_replace<add_to_render_tag>(entity);
    return entity;
}

using Point       = std::array<double, 2>;
using ear_Polygon = std::vector<std::vector<Point> >;

void triangulateSlice(const manifold::Polygons &manifoldPolys) {
    // 2. 转换 manifold 数据到 earcut 格式
    ear_Polygon polygon;
    for (const auto &ring: manifoldPolys) {
        std::vector<Point> earcut_ring;
        for (const auto &p: ring) {
            earcut_ring.push_back({(double) p.x, (double) p.y});
        }
        polygon.push_back(earcut_ring);
    }

    // 3. 执行三角化
    // 返回的是顶点索引，每 3 个索引代表一个三角形
    std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(polygon);

    // 4. 渲染逻辑 (伪代码)
    // for (size_t i = 0; i < indices.size(); i += 3) {
    //     drawTriangle(polygon_flattened[indices[i]], ...);
    // }
}

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


struct glyph {
    uint32_t unicode = 0;
    float advance    = 0.0f; // 表示渲染完这个字符后，光标应该向右移动多远来放置下一个字符

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
    std::map<uint32_t, glyph> glyphs;
};


#include <iostream>
#include "json.hpp"

using json = nlohmann::json;

void read_msdf_atlas(Msdf_text &msdf_text, std::string file_path) {
    // 1. 打开文件流
    std::ifstream file(file_path);

    if (!file.is_open()) {
        std::cerr << "无法打开文件！" << std::endl;
        return;
    }
    try {
        // 2. 直接从流解析
        json data = json::parse(file);

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
            struct glyph tem;
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
    } catch (json::parse_error &e) {
        std::cerr << "JSON 语法错误: " << e.what() << std::endl;
    }
}


#include <msdfgen.h>
#include <msdfgen-ext.h> // 该头文件包含了加载字体所需的 FreetypeHandle


int main(int argc, char *argv[]) {
    Msdf_text msdf_text;
    read_msdf_atlas(msdf_text, "atlas.json");

    msdfgen::FreetypeHandle *ft = msdfgen::initializeFreetype();

    msdfgen::FontHandle *font = loadFont(ft, "/Users/panxin/Library/Fonts/JetBrainsMonoNL-Regular.ttf");
    if (!font) {
        deinitializeFreetype(ft);
        return -1;
    }
    msdfgen::Shape shape;
    if (loadGlyph(shape, font, 'A', msdfgen::FONT_SCALING_EM_NORMALIZED)) {
        // 预处理：标准化轮廓方向
        shape.normalize();

        // 为边分配颜色（MSDF 的核心步骤，确保角点锐利）
        edgeColoringByDistance(shape, 3.0);

        float size_of_msdf = 32;


        // 4. 配置输出位图 (32x32 像素)
        msdfgen::Bitmap<float, 3> msdf(size_of_msdf, size_of_msdf);

        // 5. 设置投影变换 (缩放和位移)
        // 参数：Projection(scale, translation), range (边缘影响范围)
        double padding = 2.0;
        msdfgen::SDFTransformation t(
                                     msdfgen::Projection(size_of_msdf,
                                                         msdfgen::Vector2(7.0 / size_of_msdf,
                                                                          4.0 / size_of_msdf + padding / size_of_msdf)),
                                     msdfgen::Range(4.0 / size_of_msdf));

        // 推荐设置：range = 2.0
        // 如果要加外发光/描边：可以设为 4.0 或更高，因为你需要额外的空间来存储边缘之外的距离信息。
        // 6. 执行 MSDF 生成核心算法


        //
        msdfgen::MSDFGeneratorConfig config;
        config.overlapSupport                    = true; // 开启重叠支持
        config.errorCorrection.mode              = msdfgen::ErrorCorrectionConfig::EDGE_PRIORITY;
        config.errorCorrection.distanceCheckMode = msdfgen::ErrorCorrectionConfig::ALWAYS_CHECK_DISTANCE;

        generateMSDF(msdf, shape, t, config);

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
    }
    // return 0;

    LOG_INFO(g_log(), "Hello from {}!", "Quill v11.0.2");
    // std::cout << " UI_component.h:111  " << std::endl; // 是文件的路径就可以在clion中直接点击显示
    auto &backend = VK_backend::get();
    backend.engine_init(); // 必须单独调用，不能在 std::call_once 中 ，否则会死锁
    init_current_descriptor_pool();


    // UI 部分有些细节做的不到位，但是还是全黑的，且没有警告提示了
    UI_block("按钮1", 0, 0, 60, 60);
    UI_block("功能块", 0, 0, 50, 200);
    UI_block("按钮2", 0, 0, 145, 130);


    // load_gltf_model("sky box", "assets/Box.gltf");
    // load_gltf_model("Damaged Helmet", "assets/DamagedHelmet.gltf");

    // 天空盒
    {
        auto entity                                     = add_sky_box("skybox");
        auto texture                                    = create_skybox_texture_all("");
        std::optional<Texture_parameter> sampler_skybox = texture;
        set_render_parameter(entity, "sampler_skybox", sampler_skybox);
        logic_update_add_tag<skybox_tag>(entity);
        // 还需再增加一个特殊的标记，用于最后绘制，UI前，所有3D 完成后
    } {
        // 创建一个球体模型
        manifold::Manifold sphere = manifold::Manifold::Sphere(10.0f);

        // 在高度 5.0 处切片
        // 返回值是一个 CrossSection 对象，内部封装了 Clipper2 库来处理二维布尔运算
        manifold::CrossSection section = sphere.Slice(5.0f);

        // 导出多边形顶点数据
        manifold::Polygons polys = section.ToPolygons();

        triangulateSlice(polys);
    }
    // 3d 模型
    {
        manifold::Manifold box = manifold::Manifold::Cube({10, 10, 10}, true);
        manifold::MeshGL mesh  = box.GetMeshGL();
    } {
        // 创建两个简单的几何体并取交集
        manifold::Manifold box = manifold::Manifold::Cube({10, 10, 10}, true);

        manifold::MeshGL mesh = box.GetMeshGL();
        mesh.numProp          = 8; // 现在每个顶点占 5 个 float (x, y, z, nx, ny, nz, u, v)
        std::vector<float> newProps;
        newProps.reserve(mesh.NumVert() * mesh.numProp); // 预留空间
        // 4. 为原有的每个顶点补充 UV 数据
        // 注意：mesh.vertProperties 原本只存了 [x0, y0, z0, x1, y1, z1...]
        for (size_t i = 0; i < mesh.vertProperties.size(); i += 3) {
            // 复制 XYZ
            newProps.push_back(mesh.vertProperties[i]);     // x
            newProps.push_back(mesh.vertProperties[i + 1]); // y
            newProps.push_back(mesh.vertProperties[i + 2]); // z

            newProps.push_back(0.0f); // nx
            newProps.push_back(0.0f); // ny
            newProps.push_back(0.0f); // nz
            // 计算并添加简单的 UV (例如根据坐标映射)
            float u = (mesh.vertProperties[i] + 5.0f) / 10.0f;
            float v = (mesh.vertProperties[i + 1] + 5.0f) / 10.0f;
            newProps.push_back(u);
            newProps.push_back(v);
        }
        mesh.vertProperties.resize(newProps.size(), 0.0f);
        for (size_t i = 0; i < newProps.size(); i++) {
            mesh.vertProperties[i] = newProps[i];
        }
        manifold::Manifold boxWithUV(mesh);

        manifold::Manifold ball = manifold::Manifold::Sphere(7, 32);

        // 使用布尔运算符
        manifold::Manifold intersected = boxWithUV + ball; // '^' 为交集, '+' 为并集, '-' 为差集

        // 导出为网格数据
        auto mesh_last = intersected.GetMeshGL(3);

        auto entity    = object_3d_model("manifold ", mesh_last, {0, 0, -50});
        uint32_t index = 7;
        set_render_parameter(entity, "samplerColor", index);
        logic_update_add_tag<opacity_tag>(entity);
    } {
        auto entity = UI_text("文字A", 200, 200, 500, 500);
        set_render_parameter(entity, "msdf", "output_A_msdf.png");
    } {
        auto value     = get_max_descriptor_update_after_bind_samplers();
        auto entity    = object_3d_model("blender Suzanne -3", "assets/suzanne.obj", {-3.0f, 0.0f, 0.0f});
        auto texture   = create_textures_to_gpu(backend, "assets/suzanne0.ktx");
        uint32_t index = add_bindless_uniform_sampler2D("assets/suzanne0.ktx", texture);
        set_render_parameter(entity, "samplerColor", index);
        logic_update_add_tag<opacity_tag>(entity);
    } {
        auto entity  = object_3d_model("blender Suzanne +3", "assets/suzanne.obj", {3.0f, 0.0f, 0.0f});
        auto texture = create_textures_to_gpu(backend, "assets/suzanne1.ktx");
        auto index   = add_bindless_uniform_sampler2D("assets/suzanne1.ktx", texture);
        set_render_parameter(entity, "samplerColor", index);
        logic_update_add_tag<opacity_tag>(entity);
    }

    render_thread_start(backend);

    register_glfw(backend.get_window());

    // Render loop
    while (!glfwWindowShouldClose(backend.get_window())) {
        glfwWaitEvents();
        if (GLFW_TRUE == glfwWindowShouldClose(backend.get_window())) {
            break;
        }
        glfwPollEvents();  // Event polling
        deal_glfw_event(); // 统一分发执行
        clean_render_entity();
        sync_render_data_to_render_thread();

        // vk_render_GPU::instance().one_cycle(backend);

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    Logic_entt().clear(); // 必须先清理， root entity 会占有一部分资源，需要先清理

    // vk_render_GPU::instance().exit_and_clean(backend);
    render_thread_stop_and_wait();

    // 全局的 push_constants 的 buffer ,最后在这里销毁稍微有点不太好。
    auto &buffer = get_uniform_buffer();
    buffer->destroy_buffer();

    backend.engine_destroy();
    backend.destroy();
}


void test_projection_matrix() {
    auto entity = object_3d_model("triangle", "", {0.0f, 0.0f, 0.0f});
    add_geometry_data(entity, {-0.5f, -0.5f, 0.0f}, {0.5f, -0.5f, 0.0f}, {0.0f, 0.5f, 0.0f});
}


void add_deferred_pass(void) {
    auto &backend      = VK_backend::get();
    const auto sampler = base_sample(); {
        const auto entity = add_render_pass("blank");
        logic_update_add_tag<deferred_pass_tag>(entity);

        Texture_parameter position_texture = {
            .image       = backend.G_buffer_Position_images_.at(0), // 之前的差一帧的会出现绿色的问题在这里
            .sampler     = sampler,
            .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
        };
        std::optional<Texture_parameter> position = position_texture;
        Texture_parameter normal_texture          = {
            .image       = backend.g_buffer_Normal_images_.at(0), // 之前的差一帧的会出现绿色的问题在这里
            .sampler     = sampler,
            .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
        };
        std::optional<Texture_parameter> normal = normal_texture;
        Texture_parameter baseColor_texture     = {
            .image       = backend.G_buffer_BaseColor_images_.at(0), // 之前的差一帧的会出现绿色的问题在这里
            .sampler     = sampler,
            .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
        };
        std::optional<Texture_parameter> baseColor = baseColor_texture;

        Point_2 temp_value = {2.0, 2.0};
        set_push_constant_parameter(entity, "frag_scale", temp_value);

        // 下面三个只能选择一个显示，问题应该再下面的函数中，而不是frag shader中
        set_render_parameter(entity, "samplerPosition", position);
        set_render_parameter(entity, "samplerNormal", normal);
        set_render_parameter(entity, "samplerBaseColor", baseColor);
        auto temp_ptr          = create_SSBO_buffer(1024 * 5);
        float color[16]        = {1.0f, 0.0f, 0.0f, 1.0f};
        auto mem_copy_function = [color](void *dst) {
            memcpy(dst, color, sizeof(color));
        };
        copy_mem_from_cpu_to_gpu(temp_ptr, mem_copy_function);
        set_render_parameter(entity, "light_buffer", temp_ptr);
    }
}
