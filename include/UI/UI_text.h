//
// Created by 潘鑫 on 2026/4/8.
//

#ifndef HELLO_MAC_UI_TEXT_H
#define HELLO_MAC_UI_TEXT_H


#include <string>
#include "base_2d_render_object.h"

/**
 * 只存储绘制需要的数据
 */
struct UI_Text_data {
    std::string render_text_;
    std::string font_file_name_;
    float char_size_;
};

struct Tag_UI_text {
};

struct Tag_UI_text_data_dirty {
};

struct Tag_UI_text_renderable {
};


// UI_Text_data


class UI_Text : public object_2d {
public:
    explicit UI_Text(const std::string &debug_name, entt::entity parent_entity = entt::null);

    // 这里把之后的绘制的部分 的 代码也放进去了
    UI_Text &set_render_text(const std::string &utf8_text, const std::string &font_file_name, float char_size);

    void add_render_text(const std::string &text);

    [[nodiscard]] const std::string &get_render_text() const;

    UI_Text &draw();

    UI_Text &update();

    UI_Text &set_renderable(bool renderable);
};


#endif //HELLO_MAC_UI_TEXT_H
