//
// Created by 潘鑫 on 2025/11/27.
//

#ifndef TEXTURE_TBO_H
#define TEXTURE_TBO_H
#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include "stb_image.h"

class Texture_logic {
public:
    std::string path_;
    std::string texture_name_;

    bool set_path(const std::string &path, const std::string &texture_name) {
        this->path_ = path;
        this->texture_name_ = texture_name;
        return true;
    }

    char const *get_texture_name() const {
        return texture_name_.c_str();
    }
};


static bool create_vertex_shader(const std::string &path,
                                 std::map<std::string, texture_and_share> *map) {
    if (path.empty() == false) {
        auto it = map->find(path);
        if (it != map->end()) {
            it->second.shared_number++;
        } else {
            unsigned int texture = 0;
            glGenTextures(1, &texture);

            int width, height, nrComponents;
            unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
            if (data) {
                GLenum format;
                switch (nrComponents) {
                    case 0:
                        return false;
                    case 1:
                        format = GL_RED;
                        break;
                    case 3:
                        format = GL_RGB;
                        break;
                    case 4:
                        format = GL_RGBA;
                        break;
                    default:
                        return false;
                }
                glBindTexture(GL_TEXTURE_2D, texture);
                glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                stbi_image_free(data);
                map->insert({path, {texture, 1}});
                return true;
            } else {
                std::cout << "Texture failed to load at path: " << path << std::endl;
                stbi_image_free(data);
                glDeleteTextures(1, &texture);
            }
            return false;
        }
    }
}

/**
 *
 * @param faces
 * @return
 */
static bool loadCubeMap(const std::string &path,
                        std::map<std::string, texture_and_share> *map) {
    if (path.empty() == false) {
        auto it = map->find(path);
        if (it != map->end()) {
            it->second.shared_number++;
        } else {
            unsigned int texture = 0;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

            GLenum format;
            int width, height, nrComponents;
            std::string name = path;

            for (int i = 0; i < 6; ++i) {
                name += std::to_string(i);
                unsigned char *data = stbi_load(name.c_str(), &width, &height, &nrComponents, 0);
                if (data) {
                    switch (nrComponents) {
                        case 0:
                            return 0;
                        case 1:
                            format = GL_RED;
                            break;
                        case 3:
                            format = GL_RGB;
                            break;
                        case 4:
                            format = GL_RGBA;
                            break;
                        default:
                            return 0;
                    }
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format,
                                 GL_UNSIGNED_BYTE,
                                 data);

                    stbi_image_free(data);
                } else {
                    std::cout << "Texture failed to load at path: " << name.c_str() << std::endl;
                    stbi_image_free(data);
                    glDeleteTextures(1, &texture);
                    return false;
                }
            }
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,
                            format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,
                            format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,
                            format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //更改这两个参数可以消除天空盒边缘的碎裂感

            map->insert({path, {texture, 1}});
            return true;
        }
    }
    return false;
}

#endif //TEXTURE_TBO_H
