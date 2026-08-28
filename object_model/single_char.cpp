//
// Created by 潘鑫 on 2026/8/3.
//

#include "single_char.h"
#include <array>
#include <iostream>
#include <vector>


#include <msdfgen.h>
#include <msdfgen-ext.h> // 该头文件包含了加载字体所需的 FreetypeHandle

#include "earcut.h"
#include "manifold/manifold.h"


void test_single_char() {
    msdfgen::FreetypeHandle *ft = msdfgen::initializeFreetype();

    msdfgen::FontHandle *font = loadFont(ft, "/Users/panxin/Library/Fonts/JetBrainsMonoNL-Regular.ttf");
    if (!font) {
        deinitializeFreetype(ft);
        return;
    }
    msdfgen::Shape shape;
    if (loadGlyph(shape, font, 'A', msdfgen::FONT_SCALING_EM_NORMALIZED)) {
        // 预处理：标准化轮廓方向
        shape.normalize();
        auto bounds = shape.getBounds();


        // 为边分配颜色（MSDF 的核心步骤，确保角点锐利）
        edgeColoringByDistance(shape, 3.0);

        // 距离场需要留白空间存储过渡渐变，否则外轮廓会被直接截断
        double distanceRange = 4.0;

        float size_of_msdf = 32;
        float scale        = 32;

        // 4. 根据 Bounds 计算目标 Bitmap 的物理宽高  需要向上对齐
        //    添加微小偏置，防止浮点数无限接近整数时因精度问题导致少算 1 像素
        int width  = static_cast<int>((bounds.r - bounds.l) * scale + 2 * distanceRange + 0.9999);
        int height = static_cast<int>((bounds.t - bounds.b) * scale + 2 * distanceRange + 0.9999);

        // 2. 进位到偶数（部分图形 API 在渲染奇数宽度的纹理时性能较差）
        if (width % 2 != 0) width++;
        if (height % 2 != 0) height++;

        // 实例化浮点型 Bitmap 容器（3通道代表包含 R, G, B 的 MSDF）  配置输出位图 (32x32 像素)
        msdfgen::Bitmap<float, 3> msdf(width, height);

        auto translate = msdfgen::Vector2(-bounds.l + distanceRange / scale,
                                          -bounds.b + distanceRange / scale);
        msdfgen::SDFTransformation transform(msdfgen::Projection(size_of_msdf, translate),
                                             msdfgen::Range(distanceRange / size_of_msdf));

        // 推荐设置：range = 2.0
        // 如果要加外发光/描边：可以设为 4.0 或更高，因为你需要额外的空间来存储边缘之外的距离信息。
        // 6. 执行 MSDF 生成核心算法


        //
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
    }
}
