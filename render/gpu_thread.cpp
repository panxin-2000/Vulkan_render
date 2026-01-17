//
// Created by 潘鑫 on 2025/2/26.
//

#include <iostream>

#include "shader_common.h"


int get_glenum_length(GLenum type) {
    switch (type) {
        case GL_BYTE:
            return 1;
        case GL_UNSIGNED_BYTE:
            return 1;
        case GL_SHORT:
            return 2;
        case GL_UNSIGNED_SHORT:
            return 2;
        case GL_INT:
            return 4;
        case GL_UNSIGNED_INT:
            return 4;
        case GL_FLOAT:
            return 4;
        case GL_2_BYTES:
            return 2;
        case GL_3_BYTES:
            return 3;
        case GL_4_BYTES:
            return 4;
        default:
            std::cout << "error get_glenum_length " << type << std::endl;
            return 0;
    }
}
