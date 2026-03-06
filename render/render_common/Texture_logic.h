//
// Created by 潘鑫 on 2025/11/27.
//

#ifndef TEXTURE_TBO_H
#define TEXTURE_TBO_H
#include <iostream>

enum texture_type {
    texture_2d,
    texture_box,
};

class Texture_logic {
public:
    std::string path_;
    std::string texture_name_;
    texture_type texture_type_;

    bool set_path(const std::string &path, const std::string &texture_name) {
        this->path_ = path;
        this->texture_name_ = texture_name;
        return true;
    }

    char const *get_texture_name() const {
        return texture_name_.c_str();
    }
};


#endif //TEXTURE_TBO_H
