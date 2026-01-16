#ifndef SHADER_H
#define SHADER_H


#define GLEW_STATIC
#include <GL/glew.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

#include "VAO_object.h"
#include "base_element/point_3.h"

class Shader_object {
public:
    GLuint shaderProgram;
    GLuint UBO = NULL_GPU_INDEX;

    Shader_object() = default;

    ~Shader_object() {
        if (shaderProgram != NULL_GPU_INDEX) {
            glDeleteShader(shaderProgram);
            shaderProgram = NULL_GPU_INDEX;
        }
        if (UBO != NULL_GPU_INDEX) {
            glDeleteBuffers(1, &UBO);
            UBO = NULL_GPU_INDEX;
        }
    }


    const char *vertexPath;
    const char *fragmentPath;
    const char *geometryPath;
    GLsizeiptr UBO_size;
    const void *UBO_data;

    enum Uniforms_type {
        gl_bool,
        gl_int,
        gl_float,
        gl_vec2,
        gl_vec3,
        gl_vec4,
        gl_mat2,
        gl_mat3,
        gl_mat4,
    };

    union data_value_or_ptr {
        bool bool_val;
        int int_val;
        float float_val;
        float vec_2[2];
        float vec_3[3];
        float vec_4[4];
        float mat_2[4];
        float mat_3[9];
        float mat_4[16];
    };

    void set_UBO_parameter(GLsizeiptr UBO_size,
                           const void *UBO_data) {
        this->UBO_size = UBO_size;
        this->UBO_data = UBO_data;
    }

    static void set_mat2_value(float *address, const uint8_t row, const uint8_t column, const float value) {
        address[column * 2 + row] = value;
    }

    static void set_mat3_value(float *address, const uint8_t row, const uint8_t column, const float value) {
        address[column * 3 + row] = value;
    }

    static void set_mat4_value(float *address, const uint8_t row, const uint8_t column, const float value) {
        address[column * 4 + row] = value;
    }

    static void set_model_transform_zoom_rotate(float *address, Point_3 zoom, Point_3 rotate, Point_3 offset) {
        set_mat4_value(address, 0, 0, zoom.x);
        set_mat4_value(address, 1, 1, zoom.y);
        set_mat4_value(address, 2, 2, zoom.z);

        set_mat4_value(address, 0, 3, offset.x);
        set_mat4_value(address, 1, 3, offset.y);
        set_mat4_value(address, 2, 3, offset.z);
        set_mat4_value(address, 3, 3, 1);
    }

    std::map<std::string, std::tuple<Uniforms_type, data_value_or_ptr, uint8_t> > uniforms_map;

    void add_uniform(const std::string &name, Uniforms_type uniforms_type, data_value_or_ptr &data,
                     uint8_t number = 1) {
        auto [it, success] =
                uniforms_map.insert({name, std::make_tuple(uniforms_type, data, number)});
        if (!success) {
            it->second = std::make_tuple(uniforms_type, data, number);
        }
    }

    void set_vertex_shader(const char *path) {
        vertexPath = path;
    }

    void set_fragment_shader(const char *path) {
        fragmentPath = path;
    }

    void set_geometry_shader(const char *path) {
        geometryPath = path;
    }

    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    void Shader_init() {
        if (vertexPath == nullptr) {
            return;
        }
        if (fragmentPath == nullptr) {
            return;
        }
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::string geometryCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        std::ifstream gShaderFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try {
            // open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
            // if geometry shader path is present, also load a geometry shader
            if (geometryPath != nullptr) {
                gShaderFile.open(geometryPath);
                std::stringstream gShaderStream;
                gShaderStream << gShaderFile.rdbuf();
                gShaderFile.close();
                geometryCode = gShaderStream.str();
            }
        } catch (std::ifstream::failure &e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }
        const char *vShaderCode = vertexCode.c_str();
        const char *fShaderCode = fragmentCode.c_str();
        // 2. compile shaders
        unsigned int vertex, fragment;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        // if geometry shader is given, compile geometry shader
        unsigned int geometry;
        if (geometryPath != nullptr) {
            const char *gShaderCode = geometryCode.c_str();
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "GEOMETRY");
        }
        // shader Program
        if (shaderProgram != NULL_GPU_INDEX)
            shaderProgram = glCreateProgram();
        else {
            glDeleteShader(shaderProgram);
            shaderProgram = NULL_GPU_INDEX;
            shaderProgram = glCreateProgram();
        }
        glAttachShader(shaderProgram, vertex);
        glAttachShader(shaderProgram, fragment);
        if (geometryPath != nullptr)
            glAttachShader(shaderProgram, geometry);
        glLinkProgram(shaderProgram);
        checkCompileErrors(shaderProgram, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if (geometryPath != nullptr)
            glDeleteShader(geometry);

        initUBO(UBO_size, UBO_data);
    }

    void initUBO(GLsizeiptr size, const void *data) {
        glGenBuffers(1, &UBO);
        glBindBuffer(GL_UNIFORM_BUFFER, UBO);
        // 分配内存并传入数据（GL_STATIC_DRAW表示数据不频繁修改）
        glBufferData(GL_UNIFORM_BUFFER, 4 * 16 * 5, data, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, NULL_GPU_INDEX);
    }

    // activate the shader
    // ------------------------------------------------------------------------
    void use_shader_program() const {
        glUseProgram(shaderProgram);
    }

    GLuint get_shader_program() const {
        return (shaderProgram);
    }


    void update_uniforms() {
        use_shader_program();
        for (auto pair: uniforms_map) {
            auto &[name, data] = pair;
            std::apply([this,name](auto &&PH2, auto &&PH3, auto &&PH4) {
                update_uniform_detail(name,
                                      std::forward<decltype(PH2)>(PH2),
                                      std::forward<decltype(PH3)>(PH3),
                                      std::forward<decltype(PH4)>(PH4));
            }, data);
        }
    }

    void update_uniform_detail(const std::string &name, Uniforms_type uniforms_type, data_value_or_ptr &data,
                               uint8_t number) {
        switch (uniforms_type) {
            case gl_bool: {
                setBool(name, data.bool_val);
                break;
            }
            case gl_int: {
                setInt(name, data.int_val);
            }
            case gl_float: {
                setFloat(name, data.float_val);
            }
            case gl_vec2: {
                setVec2(name, data.vec_2, number);
            }
            case gl_vec3: {
                setVec3(name, data.vec_3, number);
            }
            case gl_vec4: {
                setVec4(name, data.vec_4, number);
            }
            case gl_mat2: {
                setMat2(name, data.mat_2, number);
            }
            case gl_mat3: {
                setMat3(name, data.mat_3, number);
            }
            case gl_mat4: {
                setMat4(name, data.mat_4, number);
            }
        }
    }


    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string &name, bool value) const {
        glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), (int) value);
    }

    // ------------------------------------------------------------------------
    void setInt(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), value);
    }

    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, float value) const {
        glUniform1f(glGetUniformLocation(shaderProgram, name.c_str()), value);
    }

    // ------------------------------------------------------------------------
    void setVec2(const std::string &name, const float *value, uint8_t number) const {
        glUniform2fv(glGetUniformLocation(shaderProgram, name.c_str()), number, value);
    }

    // ------------------------------------------------------------------------
    void setVec3(const std::string &name, const float *value, uint8_t number) const {
        glUniform3fv(glGetUniformLocation(shaderProgram, name.c_str()), number, value);
    }

    // ------------------------------------------------------------------------
    void setVec4(const std::string &name, const float *value, uint8_t number) const {
        glUniform4fv(glGetUniformLocation(shaderProgram, name.c_str()), number, value);
    }

    // ------------------------------------------------------------------------
    void setMat2(const std::string &name, const float *value, uint8_t number) const {
        glUniformMatrix2fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, GL_FALSE, value);
    }

    // ------------------------------------------------------------------------
    void setMat3(const std::string &name, const float *value, uint8_t number) const {
        glUniformMatrix3fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, GL_FALSE, value);
    }

    // ------------------------------------------------------------------------
    void setMat4(const std::string &name, const float *value, uint8_t number) const {
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, GL_FALSE, value);
    }

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(GLuint shader, std::string type) {
        GLint success;
        GLchar infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog <<
                        "\n -- --------------------------------------------------- -- " << std::endl;
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog <<
                        "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};
#endif
