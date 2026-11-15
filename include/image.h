//
// Created by 潘鑫 on 2026/5/14.
//

#ifndef HELLO_MAC_IMAGE_H
#define HELLO_MAC_IMAGE_H
#include <vector>
#include <algorithm>
#include <iostream>
#include <ostream>
#include <string>
#include "stb_image.h"

inline unsigned char floatToByte(const float value) {
    // std::lround 会自动进行精确的四舍五入，并返回 long 类型
    // 依然需要 clamp 是为了防止超限输入（如负数或大于 1.0 的值）
    return static_cast<unsigned char>(std::clamp(std::lround(value * 255.0f), 0L, 255L));
}

class Image {
    struct RGBA {
        uint8_t r, g, b, a;
    };

    uint8_t *image_data_ = nullptr;
    size_t width_        = 0;
    size_t height_       = 0;
    int channels_        = 4;

public:
    bool init_single_color(const uint8_t &R, const uint8_t &G, const uint8_t &B, const uint8_t &A = 0) {
        image_data_ = static_cast<uint8_t *>(malloc(2 * 2 * 4));
        width_      = 2;
        height_     = 2;
        if (image_data_ == nullptr) {
            return false;
        }
        write(0, 0, R, G, B, A);
        write(0, 1, R, G, B, A);
        write(1, 0, R, G, B, A);
        write(1, 1, R, G, B, A);
        return true;
    }


    bool init(const size_t width, const size_t height) {
        image_data_ = static_cast<uint8_t *>(malloc(width * height * 4));
        width_      = width;
        height_     = height;
        if (image_data_ == nullptr) {
            return false;
        }
        return true;
    }

    bool init(uint8_t *image_data, const size_t width, const size_t height, int channels) {
        if (image_data != nullptr) {
            image_data_ = static_cast<uint8_t *>(malloc(width * height * channels));
            memcpy(image_data_, image_data, width * height * channels);
        }
        width_    = width;
        height_   = height;
        channels_ = channels;
        if (image_data_ == nullptr) {
            return false;
        }
        return true;
    }

    bool init(const std::string &filename) {
        int temp_channels = 4;
        image_data_       = stbi_load(filename.c_str(),
                                (int *) &width_,
                                (int *) &height_,
                                &temp_channels,
                                4);
        if (image_data_ == nullptr) {
            std::cerr << "Failed to load image: " << filename << std::endl;
            return false;
        }
        return true;
    }

    bool init(stbi_uc const *buffer, int len) {
        int temp_channels = 4;
        image_data_       = stbi_load_from_memory(buffer,
                                            len,
                                            (int *) &width_,
                                            (int *) &height_,
                                            &temp_channels,
                                            4);
        if (image_data_ == nullptr) {
            std::cerr << "Failed to load image: " << std::endl;
            return false;
        }
        return true;
    }


    ~Image() {
        if (image_data_ != nullptr) free(image_data_);
    }

    uint8_t *get_data() const {
        return image_data_;
    }

    [[nodiscard]] size_t get_width() const {
        return width_;
    }

    [[nodiscard]] size_t get_height() const {
        return height_;
    }

    [[nodiscard]] size_t get_channels() const {
        return channels_;
    }

    [[nodiscard]] size_t get_size() const {
        return width_ * height_ * channels_;
    }


    bool write(const size_t x, const size_t y, const float r, const float g, const float b, const float a = 0) {
        return write(x, y, floatToByte(r), floatToByte(g), floatToByte(b), floatToByte(a));
    }

    bool write(const size_t x, const size_t y, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 0) {
        if (x < width_ && y < height_) {
            *(image_data_ + 4 * (x + width_ * y) + 0) = r;
            *(image_data_ + 4 * (x + width_ * y) + 1) = g;
            *(image_data_ + 4 * (x + width_ * y) + 2) = b;
            *(image_data_ + 4 * (x + width_ * y) + 3) = a;
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
        for (int j = 0; j < height_; j++) {
            for (int i = 0; i < width_; i++)
                fprintf(f, "%d %d %d\n",

                        *(image_data_ + 4 * (i + width_ * j) + 0),
                        *(image_data_ + 4 * (i + width_ * j) + 1),
                        *(image_data_ + 4 * (i + width_ * j) + 2));
        }
        fclose(f);
        return true;
    }
};

#endif //HELLO_MAC_IMAGE_H
