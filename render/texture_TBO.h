//
// Created by 潘鑫 on 2025/11/27.
//

#ifndef TEXTURE_TBO_H
#define TEXTURE_TBO_H
#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include "stb_image.h"

class Texture_TBO {
    GLuint TBO = 0;
    std::string path;
    std::string texture_name;

public:
    void bind() {
        if (TBO != 0) {
            glBindTexture(GL_TEXTURE_2D, TBO);
        }
    }

    bool set_path(char const *path, char const *texture_name) {
        this->path = path;
        this->texture_name = texture_name;
    }

    char const *get_texture_name() const {
        return texture_name.c_str();
    }


    bool loadTexture() {
        glGenTextures(1, &TBO);
        int width, height, nrComponents;
        unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
        if (data) {
            GLenum format;
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
            glBindTexture(GL_TEXTURE_2D, TBO);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        } else {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }
        return true;
    }
};

#endif //TEXTURE_TBO_H
