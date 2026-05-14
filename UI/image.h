//
// Created by 潘鑫 on 2026/5/14.
//

#ifndef HELLO_MAC_IMAGE_H
#define HELLO_MAC_IMAGE_H
#include <vector>
#include <algorithm>
#include <cmath>

inline unsigned char floatToByte(const float value) {
    // std::lround 会自动进行精确的四舍五入，并返回 long 类型
    // 依然需要 clamp 是为了防止超限输入（如负数或大于 1.0 的值）
    return static_cast<unsigned char>(std::clamp(std::lround(value * 255.0f), 0L, 255L));
}

class Image {
    std::vector<uint8_t> data;
    size_t width_  = 0;
    size_t height_ = 0;

public:
    void init(const size_t width, const size_t height) {
        data.reserve(width * height * 4);
        width_  = width;
        height_ = height;
    }

    bool write(const uint x, const uint y, const float r, const float g, const float b, const float a = 0) {
        return write(x, y, floatToByte(r), floatToByte(g), floatToByte(b), floatToByte(a));
    }

    bool write(const uint x, const uint y, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 0) {
        if (x < width_ && y < height_) {
            data[(x + width_ * y) * 4 + 0] = r;
            data[(x + width_ * y) * 4 + 1] = g;
            data[(x + width_ * y) * 4 + 2] = b;
            data[(x + width_ * y) * 4 + 3] = a;
            return true;
        }
        return false;
    }

    [[nodiscard]] bool write_to_file(const std::string &filename) const {
        FILE *f = fopen((filename + ".ppm").c_str(), "w"); // Write image to PPM file.
        if (f == nullptr) {
            // 需要添加一个log
            return false;
        }
        fprintf(f, "P3\n%lu %lu\n%d\n", width_, height_, 255);
        for (int i = 0; i < width_; i++)
            for (int j = 0; j < height_; j++) {
                fprintf(f, "%d %d %d\n", data[i + j * width_ + 0], data[i + j * width_ + 1], data[i + j * width_ + 2]);
            }
        fclose(f);
        return true;
    }
};

#endif //HELLO_MAC_IMAGE_H
