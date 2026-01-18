#ifndef SHADER_H
#define SHADER_H


#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

#include "shader_common.h"
#include "logic_render_data.h"
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
        set_mat4_value(address, 1, 1, -zoom.y);
        set_mat4_value(address, 2, 2, zoom.z);

        set_mat4_value(address, 0, 3, offset.x);
        set_mat4_value(address, 1, 3, offset.y);
        set_mat4_value(address, 2, 3, offset.z);
        set_mat4_value(address, 3, 3, 1);
    }


    static void create_vertex_shader(const std::string &path,
                                     std::map<std::string, shader_and_share> *map) {
        if (path.empty() == false) {
            auto it = map->find(path);
            if (it != map->end()) {
                it->second.shared_number++;
            } else {
                std::string shader_code;
                try {
                    std::ifstream file;
                    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
                    file.open(path);
                    std::stringstream shader_stream;
                    shader_stream << file.rdbuf();
                    file.close();

                    shader_code = shader_stream.str();
                } catch (std::ifstream::failure &e) {
                    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
                }
                const char *vShaderCode = shader_code.c_str();
                unsigned int shader = glCreateShader(GL_VERTEX_SHADER);
                glShaderSource(shader, 1, &vShaderCode, NULL);
                glCompileShader(shader);
                checkCompileErrors(shader, "VERTEX");
                map->insert({path, {shader, 1}});
                // 创建EBO
            }
        }
    }

    static void create_fragment_shader(const std::string &path,
                                       std::map<std::string, shader_and_share> *map) {
        if (path.empty() == false) {
            auto it = map->find(path);
            if (it != map->end()) {
                it->second.shared_number++;
            } else {
                std::string shader_code;
                try {
                    std::ifstream file;
                    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
                    file.open(path);
                    std::stringstream shader_stream;
                    shader_stream << file.rdbuf();
                    file.close();

                    shader_code = shader_stream.str();
                } catch (std::ifstream::failure &e) {
                    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
                }
                const char *vShaderCode = shader_code.c_str();
                unsigned int shader = glCreateShader(GL_FRAGMENT_SHADER);
                glShaderSource(shader, 1, &vShaderCode, NULL);
                glCompileShader(shader);
                checkCompileErrors(shader, "FRAGMENT");
                map->insert({path, {shader, 1}});
                // 创建EBO
            }
        }
    }

    static void create_geometry_shader(const std::string &path,
                                       std::map<std::string, shader_and_share> *map) {
        if (path.empty() == false) {
            auto it = map->find(path);
            if (it != map->end()) {
                it->second.shared_number++;
            } else {
                std::string shader_code;
                try {
                    std::ifstream file;
                    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
                    file.open(path);
                    std::stringstream shader_stream;
                    shader_stream << file.rdbuf();
                    file.close();

                    shader_code = shader_stream.str();
                } catch (std::ifstream::failure &e) {
                    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
                }
                const char *vShaderCode = shader_code.c_str();
                unsigned int shader = glCreateShader(GL_GEOMETRY_SHADER);
                glShaderSource(shader, 1, &vShaderCode, NULL);
                glCompileShader(shader);
                checkCompileErrors(shader, "GEOMETRY");
                map->insert({path, {shader, 1}});
                // 创建EBO
            }
        }
    }


    void shader_init_and_attach(logic_render_data *render,
                                std::map<std::string, shader_and_share> *vertex_shader_map,
                                std::map<std::string, shader_and_share> *fragment_shader_map,
                                std::map<std::string, shader_and_share> *geometry_shader_map) {
        // shader Program
        if (render == nullptr)
            return;
        if (shaderProgram == NULL_GPU_INDEX)
            shaderProgram = glCreateProgram();
        else {
            glDeleteShader(shaderProgram);
            shaderProgram = NULL_GPU_INDEX;
            shaderProgram = glCreateProgram();
        } {
            auto it = vertex_shader_map->find(render->vertexPath_);
            if (it != vertex_shader_map->end()) {
                glAttachShader(shaderProgram, it->second.shader);
            }
        } {
            auto it = fragment_shader_map->find(render->fragmentPath_);
            if (it != fragment_shader_map->end()) {
                glAttachShader(shaderProgram, it->second.shader);
            }
        } {
            auto it = geometry_shader_map->find(render->geometryPath_);
            if (it != geometry_shader_map->end()) {
                glAttachShader(shaderProgram, it->second.shader);
            }
        }
        glLinkProgram(shaderProgram);
        checkCompileErrors(shaderProgram, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        initUBO();
    }

    void initUBO() {
        glGenBuffers(1, &UBO);
        glBindBuffer(GL_UNIFORM_BUFFER, UBO);
        // 分配内存并传入数据（GL_STATIC_DRAW表示数据不频繁修改）
        glBufferData(GL_UNIFORM_BUFFER, 4 * 16 * 5, nullptr, GL_STATIC_DRAW);
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


    void update_uniforms(std::map<std::string, std::tuple<Uniforms_type, data_value_or_ptr, uint8_t> > *uniforms_map) {
        use_shader_program();
        for (auto pair: *uniforms_map) {
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
    static void checkCompileErrors(GLuint shader, std::string type) {
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
