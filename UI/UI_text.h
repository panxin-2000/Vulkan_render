//
// Created by 潘鑫 on 2026/4/8.
//

#ifndef HELLO_MAC_UI_TEXT_H
#define HELLO_MAC_UI_TEXT_H


#include "Rect_2D_component.h"
#include <json_fwd.hpp> // 正确：只引入前置声明，文件极小，编译极快 , 只是声明
#include "base_2d_render_object.h"
#include "font_manager.h"


class UI_Text : public object_2d {
public:
    UI_Text(const std::string &name);

    // 这里把之后的绘制的部分 的 代码也放进去了
    void set_render_text(const std::string &text);

    void add_render_text(const std::string &text) {
        render_text = render_text + "\n" + text;
    }

private:
    std::string render_text;
};




void read_msdf_atlas(Msdf_text &msdf_text, std::string file_path);

entt::entity UI_text(const std::string &name,
                     float min_x,
                     float min_y,
                     float max_x,
                     float max_y);


#endif //HELLO_MAC_UI_TEXT_H
